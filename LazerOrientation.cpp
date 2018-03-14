#include "LazerOrientation.h"
#include <gpstk/SP3Data.hpp>
#include <gpstk/SP3Stream.hpp>

LazerOrientation::LazerOrientation()
{

}

void LazerOrientation::getFromSP3(char* fname)
{
    gpstk::SP3Stream file(fname);
}
