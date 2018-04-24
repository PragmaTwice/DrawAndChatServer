#include "drawinfo.h"

DrawInfo::DrawInfo(const QString &inAuthor, int inState, const QVariantMap &inArguments) :
    author(inAuthor),
    paintState(inState),
    arguments(inArguments)
{
}

DrawInfo::~DrawInfo()
{
}
