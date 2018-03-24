#ifndef LAZERORIENTATION_H
#define LAZERORIENTATION_H


class LazerOrientation
{
    public:
        LazerOrientation();
        ~LazerOrientation();

        void getFromILRS(char*);

        double azimuthGrad;
        double heightGrad;
};

#endif // LAZERORIENTATION_H
