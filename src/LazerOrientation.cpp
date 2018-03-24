#include "LazerOrientation.h"

LazerOrientation::LazerOrientation() = default;
LazerOrientation::~LazerOrientation() = default;

void LazerOrientation::getFromILRS(char* fname)
{
    gpstk::SP3Stream file(fname);
}
