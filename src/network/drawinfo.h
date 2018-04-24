#ifndef DRAWINFO_H
#define DRAWINFO_H

#include <QVariantMap>

class DrawInfo
{

public:

    QString author;
    int paintState;
    QVariantMap arguments;

    DrawInfo(const QString& inAuthor, int inState, const QVariantMap& inArguments = QVariantMap());
    ~DrawInfo();
};

#endif // DRAWINFO_H
