PunaKynä source explained.

PunaKynä is accompanied by Poppler "pdf to txt" and MathCheck programs. MathCheck is a CAS developed by Antti Valmari, and releases with LGPL license.
PunaKynä may be instructed to use MathCheck for grading an exam made with MathCheck's exam-mode, where the students answer html file is first feeded
to MathCheck, and the output is then showed to user. Pdf-to-txt program is just a small container program to feed a given pdf to a Poppler function for
translating. It was made just because at the time, I wasn't aware if Macs had any applicable commandline function built-in. On my Windows machine,
I had some trouble installing Poppler for compiling the code, but I found out that Windows has a usable commandline feature for pdf to txt, so Windows
releases don't need, nor use, Poppler.

! The QT-project file in main source directory is a combination project, that compiles all 3 projects. In most all cases, this can be ignored, and you only want
to compile the project in the PunaKyna directory.

I've been lazy with comments, though tried to be explanatory with naming practices, so let me clarify the flow of the program here.

MainWindow is the mainwindow, and works as a controller between the two main parts of the program: Browser and Grader.
These two are made with strong object orientation in mind, so sometimes they need some information from each other, and those
requests have to go through MainWindow, neither knows anything about each other, nor do they have access to same files.
MainWindow also holds the document viewer class, that deals with showing documents.

Browser is am model/view component without controller. The model is the only class that looks at the exam directory, and creates the initial exam-log.
It offers basic getters and setters, with enumerated RoleSlot (id, printname, status etc) to indicate what kind of data is needed.

Grader is a hierarchal line of classes, where GradingWindow is the head class, that maintains the exam-log, and has a GradingElementWindow for each question in the exam.
GradingElementWindow maintains its own question-log, and has a GradingElement for each comment in the question.
GradingPrinter is outside of this hierarchy, and handles the meticulous process of putting all of the data together for outputting. In order to be cancellable, printing
functions in Grading classes take a boolean value, which if false, means no changes are to be made to the files. So if false, data is gathered, if true, data is gathered
and saved.

Settings holds all (or at least most) the settings that the user may change. It uses a normal ini-format, and creates the file in an user-reachable location even on a
Mac. At the time I tried to figure a good way to let all classes to use it, for example making in global, but I couldn't figure out a way that was easy and in accordance to good programming
practices, but to let each class have a pointer to it, and give it to each class they create.

I hope that with these instructions you can follow the programs flow. To satisfy my vision, I've had to jump some probably unnecessary hoops and made the code a bit too
convoluted, but at least I did it.

--
Valtteri Laaksonen
