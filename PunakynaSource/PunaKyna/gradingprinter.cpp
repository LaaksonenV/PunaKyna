#include "gradingprinter.h"
#include "settings.h"
#include "binaryparser.h"
#include "csvparser.h"
#include "smalldialogs.h"
#include "gradingelementwindow.h"

#include <QtXlsx>
#include <QDateTime>
#include <QMimeData>
#include <QMessageBox>

GradingPrinter::GradingPrinter(Settings *settings,
                               CSVParser *file,
                               GradingElementWindow *from,
                               QObject *parent)
    : QObject(parent)
    , _examFile(file)
    , _element(from)
    , _settings(settings)
{

}

void GradingPrinter::exportToGrading(QString fileName)
{
    CSVParser output(fileName);
    if (output.rowCount() < 2)
        return;

    bool exam = true;
    int idCol = output.findExactColumn(tr("Tunniste"),0);
    if (idCol >= 0)
    {
        QMessageBox::critical(nullptr, tr("Lokalisointi ongelma"),
                              tr("Moodlen lokalisointi ei toimi kuin pitäisi."
                                 "Arviointitiedosto pitää ladata ja palauttaa "
                                 "englanninkielisellä Moodlella."));
        return;
    }

    idCol = output.findExactColumn(tr("Identifier"),0);
    if (idCol < 0)
    {
        idCol = output.findExactColumn("exam id",0);
        if (idCol < 0)
        {
            csvError();
            return;
        }
    }
    else
        exam = false;

    int stIdCol = output.findExactColumn(tr("ID number"),0);
    if (stIdCol < 0)
        stIdCol = output.findExactColumn("student id",0);
    if (stIdCol < 0)
    {
        csvError();
        return;
    }

    int comCol = output.findExactColumn(tr("Feedback comments"),0);
    if (comCol < 0)
        comCol = output.findExactColumn("feedback",0);
    if (comCol < 0)
    {
        csvError();
        return;
    }

    int scrCol = output.findExactColumn(tr("Grade"),0);
    if (scrCol < 0)
        scrCol = output.findExactColumn("total score",0);
    if (scrCol < 0)
    {
        csvError();
        return;
    }

//    QRegularExpression relevantId("tut\\.fi.(d+)");
  //  QRegularExpressionMatch relevant;

    QString MoodleIdAppend = tr("Participant ");


    // Log what is graded to information after grading
    // (0) rows in outputfile (how many to grade)
    // (1) empty answers
    // (2) wrong formats
    // (3) other automatic zeros
    // (4) amount of graded
    // (5) amount of not graded
    // (6) how many students graded more than once
    QVector<int> log(7,0);

    QString mstring = _examFile->getData(0,0);
    int maxPoints = mstring.section(":",1,1).toInt();

    // Since grades are allocated after all points are collected,
    // next will give temporary points -> students rows who got points
    QMap<int, int> outputRowPoints;
    // Count the amount of students with (index) points
    QVector<int> totalPointsforGrade(maxPoints+1);

    // Collect points for each question fo student
    QMap<QString, QVector<float> > studentsPoints;

    QVector<float> points;

    QString student;
    QString commentOutText;
    QStringList completedComment;
    float flpoint;
    float pointsForAllQ;

    log[0] = output.rowCount()-1;
    //
    // Iterate over output file to find the students to grade
    //
    for (int outputRow = 1; outputRow < output.rowCount(); ++outputRow)
    {
        // Fetch student ids
        pointsForAllQ = -1;
        student = output.getData(outputRow, idCol);

/*        if (exam)
        {
            student = output.getData(outputRow, stIdCol);

            relevant = relevantId.match(student);
            if (!relevant.hasMatch())
            {
                student = relevant.captured(1);
            }
            else
                qWarning() << "No Id found in gradings";

            student.append('-').append(output.getData(outputRow, idCol));
        }
*/
        if (!exam)
        {
//            student = output.getData(outputRow, idCol);
            student.remove(MoodleIdAppend);
        }

        // Create commentfield, but don't save completion
        completedComment = completeComments(student, false);
        if (completedComment.isEmpty())
        {
            // If there are no comments, move along to next student
            ++log[5];
            continue;
        }

        points.clear();
        // completed comments have the comment text at 0, and rest are
        // questionpoints singled
        for (int i = 1; i < completedComment.count(); ++i)
        {
            flpoint = completedComment.at(i).toFloat();
            if (flpoint < 0)
            {
                if (flpoint < float(-1.99))
                    ++log[1];
                else
                    ++log[2];
                points << 0;
            }
            else
            {
                if (pointsForAllQ < 0)
                    pointsForAllQ = 0;
                points << flpoint;
                pointsForAllQ += flpoint;
            }
        }


        studentsPoints.insert(student, points);

        commentOutText = completedComment.at(0);
        ++log[4];


        if (pointsForAllQ > maxPoints)
            pointsForAllQ = maxPoints;
        if (pointsForAllQ >= 0)
            ++totalPointsforGrade[int(floor(pointsForAllQ+float(0.01)))];
        outputRowPoints.insertMulti(int(floor(pointsForAllQ+float(0.01)))
                                    , outputRow);

        if (pointsForAllQ < 0)
            pointsForAllQ = 0;

        //
        // As EXAMs csv-parsing keeps changing:
        if (!string2Bool(_settings->getValue(
                             Settings::Section_EXAM,"allow_commas")))
        {
            commentOutText.replace(",",
                           _settings->getValue(
                                      Settings::Section_EXAM,"replace_commas"));
        }

        // Print the comment
        output.changeData(commentOutText, outputRow, comCol);



        // Print the score
        commentOutText = QString::number(double(pointsForAllQ),'g',2);
        if (exam)
        {
            commentOutText.append(" / ");
            commentOutText.append(QString::number(maxPoints));
        }
        else
        {
            output.changeData(QString::number(maxPoints), outputRow, scrCol+1);
            commentOutText.replace('.',',');
        }
        output.changeData(commentOutText, outputRow, scrCol);

    }

    QStringList gradingData = mstring.section(":",0,0)
            .split('<',QString::SkipEmptyParts);
    QVector<int> gradeLimits;
    if (exam)
    {
        if (gradingData.isEmpty())
            for (int i = 1; i < 6; ++i)
                gradeLimits << (i*(maxPoints)/6)+1;
        else
        {
            for (int i = 0; i < gradingData.count();++i)
                gradeLimits << gradingData.at(i).toInt();
            //       gradeLimits.last() = maxPoints+1;
        }

        if (!SmallDialogs::Grading::getGradings(gradeLimits,
                                                totalPointsforGrade))
            return;
    }

    _examFile->printBackupCSV(".GradingBCKUP");

    foreach (QString stdnt, studentsPoints.keys())
    {
        completeComments(stdnt, true);
    }

    QMap<QString, int> studentsGrades;

        gradingData.clear();
        for (int i = 0; i < gradeLimits.count(); ++i)
            gradingData << QString::number(gradeLimits.at(i));
        _examFile->changeData(gradingData.join('<') + QString(":")
                              + QString::number(maxPoints),0,0);

        // grade for students. Note that while student grades contains every
        //student, studentPoints contains just those with something commented
        int pointsForSingleQ = -1;
        int limit;
        for (int grade = 0; grade < gradeLimits.count()+1; ++grade)
        {
            if (grade == gradeLimits.count())
                limit = maxPoints+1;
            else
                limit = gradeLimits.at(grade);

            while (pointsForSingleQ < limit)
            {
                foreach (int row, outputRowPoints.values(pointsForSingleQ))
                {
                    student = output.getData(row, stIdCol);
                    student.append('-').append(output.getData(row, idCol));

                    if (exam)
                    {
                        output.changeData(QString::number(grade), row, 1);
                    }
                    else
                    {
                        student.remove(MoodleIdAppend);
                    }

                    studentsGrades.insert(student, grade);
                }
                ++pointsForSingleQ;
            }
        }

        QDateTime datet = QDateTime::currentDateTime();
        fileName.replace(".csv", QString("_Excl") + datet.toString
                         ("yyyy.MM.dd_hh.mm")
                         + QString(".xlsx"));

        QXlsx::Document xlsx(fileName,this);
        /*    int grade;
    for (int i = 0; true; ++i)
    {
        student = xlsx.read(i,0);
        xlsx.write(i,0,"");
        grade = xlsx.read(i,1).toInt();
        xlsx.write(i,1,"");

        if (student.isEmpty())
            break;
        if (studentGrades.contains(studentNmb))
        {
            if (studentGrades.value(studentNmb) < grade)
                studentGrades[studentNmb] = grade;
        }
        else
            studentGrades.insert(student.toInt(), grade);
    }*/ //Jos aina luodaan uusi niin ei tarvita, palauta jos luetaan vanha
        xlsx.write(1,1,QVariant(tr("Opiskelija")));
        if (exam)
            xlsx.write(1,2,QVariant(tr("Arvosana")));
        xlsx.write(1,3,QVariant(tr("Tehtävien pisteet")));

        for (int row = 0; row < studentsGrades.keys().count(); ++row)
        {
            student = studentsGrades.keys().at(row);

            xlsx.write(row+2, 1,QVariant(student.section('-',0,0)));
            if (exam)
                xlsx.write(row+2, 2,QVariant(studentsGrades.value(student)));
//            else
                student = student.section('-',1);

            if (studentsPoints.contains(student))
            {
                points = studentsPoints.value(student);
                for (int p = 0; p < points.count(); ++p)
                {
                    xlsx.write(row+2, p+3, QVariant(points.at(p)));

                }
            }
            else xlsx.write(row+2, 3, QVariant("0"));
        }

        xlsx.save();
        QFile(fileName).setPermissions(QFileDevice::ReadOwner |
                                       QFileDevice::WriteOwner |
                                       QFileDevice::ExeOwner |
                                       QFileDevice::ReadUser |
                                       QFileDevice::WriteUser |
                                       QFileDevice::ExeUser |
                                       QFileDevice::ReadGroup);


    output.printCSV(false);

    SmallDialogs::GradeLog logDial(log);
    if (logDial.exec())
    {
        //printlog
    }
}

QStringList GradingPrinter::completeComments(const QString &answer, bool actual)
{
    QStringList single;
    QMap<int, QStringList> singles;

    int qnum;
    int row = 0;
    int qtop = 0;
    float totalpoints = 0;
    if (answer.isEmpty())
        row = -1;
    else
        row= _examFile->findRow(answer, 0, 1);
    if (row < 0)
        return QStringList();

    for (int i = 1; i < _examFile->columnCount(); ++i)
    {
        single = lockComments(i, row, GradingState::Sent, actual);
        if (single.isEmpty())
            continue;
        qnum = single.takeFirst().toInt();
        singles.insert(qnum, single);
        if (qtop < qnum)
            qtop = qnum;

        if (single.at(1).toFloat() > 0)
            totalpoints += single.at(1).toFloat();
    }
    if (!singles.count())
        return QStringList();

    single.clear();
    for (int i = 0; i <= qtop; ++i)
    {
        if (singles.contains(i))
        {
            if (!single.count())
                single << singles.value(i).at(0);
            else
                single[0] += singles.value(i).at(0);
            single << singles.value(i).at(1);
        }
        else
            single << "";
    }

    if (singles.count() > 1 || !singles.contains(0))
    {
        single[0] += (QString(tr(" Kokonaispisteet: "))
                + QString::number(double(totalpoints),'g', 2)
                + QString(tr("p.")));
    }

    return single;
}

QStringList GradingPrinter::lockComments(int qCol, int aRow, int lockState,
                                    bool actual)
{
    QString dat = _examFile->getData(aRow, qCol);
    if (dat.isEmpty())
        return QStringList();

    QStringList studentData = dat.split(Printing::separator);

    QString qName = _examFile->getData(0,qCol).section('-',0,-2);
    QString qNum = _examFile->getData(0,qCol).section('-',-1,-1);
    QString sName = _examFile->getData(aRow, 0);
    QStringList retlst(qNum);
    int state = GradingState::Commented;
    fillDataRow(studentData);
    state = GradingState::charToState(
                    studentData.at(ExamFile::CommentCell_Status));

    /*
     * If the answer is already locked, we don't need to change anythin but
     * it's state if...
     */
    if ( state >= GradingState::Locked)
    {
        /*
         * ... we are asked to open it.
         */
        if (lockState < GradingState::Locked)
        {
            if (!studentData.at(ExamFile::CommentCell_Ids).isEmpty()
             || !studentData.at(ExamFile::CommentCell_Free).isEmpty())
                studentData.replace(ExamFile::CommentCell_Status,
                                    GradingState::stateToChar(
                                        GradingState::Commented));
            else
                studentData.replace(ExamFile::CommentCell_Status,
                                    GradingState::stateToChar(
                                        GradingState::NullState));
            if (actual)
            {
                _examFile->changeData(studentData.join(Printing::separator),
                                      aRow, qCol);
                emit stateChanged(qName, sName, GradingState::Commented);
            }
            return QStringList();
        }
        /*
         * ... it was locked and now must be sent
         */
        if (state == GradingState::Locked &&
                lockState == GradingState::Sent)
        {
            studentData.replace(ExamFile::CommentCell_Status,
                                GradingState::stateToChar(
                                    GradingState::Sent));
            if (actual)
            {
                _examFile->changeData(studentData.join(Printing::separator),
                                      aRow, qCol);
                emit stateChanged(qName, sName, GradingState::Sent);
            }
        }
        /*
         * Otherwise no change is needed, just return comment
         */
        retlst << studentData.at(ExamFile::CommentCell_Free)
               << studentData.at(ExamFile::CommentCell_Points);
        return retlst;
    }
    /*
     * If the answer hasn't been commented at all, locking is not needed,
     * and if it isn't locked and opening is asked...
     */
    else if (state < GradingState::AutoCommented ||
             lockState < GradingState::Locked)
        return QStringList();

    QString ret;

    if (qNum != "0")
    {
        ret = tr("T");
        ret += qNum;
        ret += ": ";
    }

    if (state == GradingState::Wrong)
    {
        ret += _settings->getValue(Settings::Section_DEF,
                                   "wrong_format")
            += " ";

        return retlst << ret << "-1";
    }
    if (state == GradingState::Empty)
    {
        ret += _settings->getValue(Settings::Section_DEF,
                                   "no_returns")
            += " ";

        return retlst << ret << "-2";
    }

    BinaryParser comments(studentData.at(ExamFile::CommentCell_Ids));
    int commentN;
    int pointsForSingleQ = studentData.at(ExamFile::CommentCell_Points).toInt();
    int pointsForSingleComm = 0;
    int divider = _element->getDivider(qName);
    if (divider == 0)
        divider = 1;

    while (comments.takeNext(commentN))
    {
        (ret += _element->getCommentText(commentN, qName)) += ", ";
        pointsForSingleComm = _element->getCommentValue(commentN, qName);
        pointsForSingleQ += pointsForSingleComm;
        if (string2Bool(_settings->getValue(Settings::Section_EXAM,
                                            "print_cpts")))
        {
            ret += (QString("(")
                + QString::number(double(pointsForSingleComm/divider), 'g', 2)
                + QString("p.), "));
        }
    }

    if (!studentData.at(ExamFile::CommentCell_Free).isEmpty())
    {
        ret += studentData.at(ExamFile::CommentCell_Free);
    }
    else
        ret.remove(ret.count()-2,2);

    float dividePoint = float(pointsForSingleQ)/divider;

    if (dividePoint > _element->getMaxPoints(qName))
        dividePoint = _element->getMaxPoints(qName);

    if (string2Bool(_settings->getValue(Settings::Section_EXAM, "print_qpts")))
    {
        if (!ret.isEmpty())
            ret += QString(", ");

        ret += (QString::number(double(dividePoint), 'g', 2)
            +  QString("p."));
    }
    else
        ret += ". ";


    if (actual)
    {
        studentData.replace(ExamFile::CommentCell_Free, ret);
        studentData.replace(ExamFile::CommentCell_Points,
                            QString::number(double(dividePoint), 'g', 2));
        studentData.replace(ExamFile::CommentCell_Ids, "");
        studentData.replace(ExamFile::CommentCell_Status,
                            GradingState::stateToChar(lockState));
        _examFile->changeData(studentData.join(Printing::separator),aRow, qCol);
        emit stateChanged(qName, sName, lockState);
    }
    if (dividePoint < 0)
        dividePoint = 0;
    retlst << ret << QString::number(double(dividePoint), 'f', 2);
    return retlst;
}

void GradingPrinter::save(int row, int column, bool ac)
{
    QStringList comm;
    bool hascomms = false;
    bool changed = false;
    QString temp;

    if (_element && row > 0 && column > 0)
    {
        comm = _examFile->getData(row, column).split(Printing::separator);
        if(comm.count() < ExamFile::CommentCell_Status ||
                GradingState::charToState(comm.at(ExamFile::CommentCell_Status))
                    < GradingState::Locked)
        {
            QString qName = _examFile->getData(0, column)
                    .section("-",0,-2);
            fillDataRow(comm);
            temp = BinaryParser::parseBinary(_element->getCommented(ac));
            if (!temp.isEmpty())
                hascomms = true;

            if (comm.at(ExamFile::CommentCell_Ids) != temp)
            {
                comm.replace(ExamFile::CommentCell_Ids, temp);
                changed = true;
            }

            temp = _element->getPersonalComment();
            if (!temp.isEmpty())
                hascomms = true;

            if (comm.at(ExamFile::CommentCell_Free) != temp)
            {
                comm.replace(ExamFile::CommentCell_Free, temp);
                changed = true;
            }

            temp = _element->getPersonalPoints();
            if (!temp.isEmpty() &&
                temp != "0")
                hascomms = true;

            if (comm.at(ExamFile::CommentCell_Points) != temp)
            {
                comm.replace(ExamFile::CommentCell_Points, temp);
                changed = true;
            }

            if ((GradingState::charToState(comm.at(ExamFile::CommentCell_Status))
                    == GradingState::AutoCommented && !ac)
                    || (ac && _element->isConflicting()))
                    changed = true;

            if (changed)
            {
                if (ac)
                {
                    comm.replace(ExamFile::CommentCell_By, "AutoCheck:" +
                                 _settings->getValue(Settings::Section_User,
                                                     "lastuser"));
                    if (_element->isConflicting())
                        comm.replace(ExamFile::CommentCell_Status,
                                     GradingState::stateToChar(
                                         GradingState::Conflicting));
                    else
                        comm.replace(ExamFile::CommentCell_Status,
                                     GradingState::stateToChar(
                                         GradingState::AutoCommented));
                }
                else
                {
                    comm.replace(ExamFile::CommentCell_By,
                                 _settings->getValue
                                 (Settings::Section::Section_User,
                                  "lastuser"));
                    if (hascomms)
                        comm.replace(ExamFile::CommentCell_Status,
                                     GradingState::stateToChar(
                                         GradingState::Commented));
                    else
                        comm.replace(ExamFile::CommentCell_Status,
                                     GradingState::stateToChar(
                                         GradingState::Viewed));
                }
                _examFile->changeData(comm.join(Printing::separator), row,
                                      column);
                // STATUS

                if (ac)
                {
                    if (_element->isConflicting())
                        emit stateChanged(qName, _examFile->getData(row, 0),
                                          GradingState::Conflicting);
                    else
                        emit stateChanged(qName, _examFile->getData(row, 0),
                                          GradingState::AutoCommented);
                }
                else if (hascomms)
                    emit stateChanged(qName, _examFile->getData(row, 0),
                                  GradingState::Commented);
                else
                    emit stateChanged(qName, _examFile->getData(row, 0),
                                  GradingState::Viewed);
            }
        }
    }
}

void GradingPrinter::fillDataRow(QStringList &d)
{
    while (d.count() < ExamFile::CommentCell_NullCell)
        d << "";
}

void GradingPrinter::csvError()
{
    QMessageBox::critical(nullptr, tr("Virhe CSV-tiedostossa"),
                          tr("CSV-tiedostosta ei löytynyt tarvittavaa"
                             " saraketta. Varmista tiedosto tai "
                             "ilmoita PunaKynän ylläpidolle"));
}
