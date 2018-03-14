#ifndef LAZERORIENTATION_H
#define LAZERORIENTATION_H


class LazerOrientation
{
    public:
        LazerOrientation();
        ~LazerOrientation();

        void getFromSP3(char*);

        double azimuthGrad;
        double heightGrad;
};

#endif // LAZERORIENTATION_H
