#ifndef MYSURVEY_H
#define MYSURVEY_H
#include <surveydialog.h>

class MySurvey : public Survey::SurveyDialog {
    Q_OBJECT

public:
    MySurvey( QWidget* parent, const char* name = 0 );
};


#endif /* MYSURVEY_H */

