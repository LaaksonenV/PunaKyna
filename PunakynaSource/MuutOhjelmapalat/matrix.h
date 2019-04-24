#ifndef MATRIX_H
#define MATRIX_H

#include <vector>
class Matrix
{
public:
    Matrix();
    Matrix(int size);
    Matrix(int x, int y);
    Matrix(const Matrix &m);

    inline Matrix eye(int size);

    bool empty();
    void makeSq();

    int height() const;
    int width()const;

    double operator ()(int x, int y) const;
    double operator ()(int x, int y, double value);

    Matrix operator +=(Matrix &m);
    Matrix operator -=(Matrix &m);
    Matrix operator *=(int i);
    Matrix operator *=(Matrix &m);

    Matrix T();

    Matrix appendrow(Matrix &m);
    Matrix appendcol(Matrix &m);


    bool SVD(Matrix &e, Matrix &E);

private:
    int maxind(int k, const Matrix &m);
    void rotate(Matrix &m, int k, l, i, j, c, s);


private:
    std::vector<std::vector<double> > mat;

};

#endif // MATRIX_H
