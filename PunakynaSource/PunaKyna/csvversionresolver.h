#ifndef CSVVERSIONRESOLVER_H
#define CSVVERSIONRESOLVER_H

class CSVParser;
#include <QString>
class Settings;

namespace v1_5_Archive
{
enum UIGradingStates : int
{
    NullState, AutoCommented, Commented, Locked, Complete
};
const QString cellSeparator = "::";
}

namespace v1_74_Archive
{
enum CommentCell
{
    CommentCell_Ids, CommentCell_Free, CommentCell_By,
    CommentCell_Status, CommentCell_NullCell
};
}

namespace v1_77_Archive
{
enum GradingStates : int
{
    UnTouched, Viewed, AutoCommented, Commented,
    Locked, Sent, NullState
};
}

namespace v1_78_Archive
{
enum MetaRow
{
    Meta_Points, Meta_AutoControl, Meta_NoControl
};
}


class CSVVersionResolver
{
public:
    enum CSVType
    {
        CSVType_Null, CSVType_Question, CSVType_Exam
    };

    CSVVersionResolver(Settings *set);

    bool resolve(CSVParser *file, CSVType type);

private:
    enum Versions
    {
        v1_5, v1_6, v1_74, v1_77, v1_78, v1_7f , Current
    };

    int checkVersion();
    bool resolveFile(int fromVersion);

private:
    Settings *_settings;
    CSVParser *_file;
    CSVType _type;
};

#endif // CSVVERSIONRESOLVER_H
