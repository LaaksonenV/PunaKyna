#include "matrix.h"

#include <math.h>

Matrix::Matrix()
    : mat(std::vector<std::vector<int> >())
{
}

 Matrix::Matrix(int size)
    : Matrix(size,size)
{
}

Matrix::Matrix(int x, int y)
    : Matrix()
{
    std::vector<int> xs = std::vector<double>(x);
    for (int iy = 0; iy < y; ++iy)
        mat.push_back(xs);
}

Matrix::Matrix(const Matrix &m)
    : Matrix(m.width(),m.height())
{
    for (int iy = 0; iy < m.height(); ++iy)
        for (int ix = 0; ix < m.width(); ++ix)
            mat[iy][ix] = m(ix,iy);
}

Matrix Matrix::eye(int size)
{
    Matrix ret(size);

    for (int i = 0; i < size; ++i)
        ret(i,i,1);
    return ret;
}

bool  Matrix::empty()
{
    return mat.empty();
}

void  Matrix::makeSq()
{
    if(empty())
        return;

    int ys = mat.size();
    int xs = mat.front().size();

    while (ys < xs)
    {
        mat.push_back(std::vector<double>(xs,0));
        ++ys;
    }
    while (xs < ys)
    {
        for (int iy = 0; iy < mat.size(); ++iy)
            mat[iy].push_back(0);
        ++ix;
    }

}

int  Matrix::height() const
{
    if(empty())
        return 0;

    return mat.size();
}

int  Matrix::width() const
{
    if(empty())
        return 0;

    return mat.front().size();
}

double Matrix::operator ()(int x, int y) const
{
    if(empty() || mat.size() <= y)
        return 0;

    std::vector<double> xs = mat.at(y);

    if (xs.size() <= x)
        return 0;

    return xs.at(x);
}

double Matrix::operator ()(int x, int y, double value)
{
    int ix = 0;
    if(!empty())
        ix = mat.front().size();
    while(ix < x)
    {
        for (int iy = 0; iy < mat.size(); ++iy)
            mat[iy].push_back(0);
        ++ix;
    }

    for(int iy = y; mat.size() <= iy; ++iy)
        mat.push_back(std::vector<int>(ix,0));

    mat[y][x] = value;
    return value;
}


Matrix  Matrix::operator +=(Matrix &m)
{
    if (height() != m.height() || width() != m.width())
        return Matrix();

    Matrix ret(width(), height());
    for (int iy = 0; iy < m.height(); ++iy)
        for (int ix = 0; ix < m.width(); ++ix)
            ret(ix,iy, mat(ix,iy) + m(ix,iy));

    return ret;
}

Matrix  Matrix::operator -=(Matrix &m)
{
    if (height() != m.height() || width() != m.width())
        return Matrix();

    Matrix ret(width(), height());
    for (int iy = 0; iy < m.height(); ++iy)
        for (int ix = 0; ix < m.width(); ++ix)
            ret(ix,iy, mat(ix,iy) - m(ix,iy));

    return ret;
}

Matrix  Matrix::operator *=(int i)
{
    Matrix ret(width(), height());
    for (int iy = 0; iy < height(); ++iy)
        for (int ix = 0; ix < width(); ++ix)
            ret(ix,iy, i*mat(ix,iy));

    return ret;
}

Matrix  Matrix::operator *=(Matrix &m)
{
    if (width() != m.height())
        return Matrix();

    Matrix ret(m.width(), height());
    double cumsum;
    for (int iy = 0; iy < height(); ++iy)
        for (int ix = 0; ix < m.width(); ++ix)
        {
            cumsum = 0;
            for (int ii = 0; ii < width(); ++ii)
                cumsum += mat(ii,iy) * m(ix,ii);

            ret(ix,iy, cumsum);
        }

    return ret;
}

Matrix  Matrix::T()
{
    Matrix ret(height(), width());
    for (int iy = 0; iy < height(); ++iy)
        for (int ix = 0; ix < width(); ++ix)
            ret(iy,ix, mat(ix,iy));

    return ret;
}

Matrix Matrix::appendrow(Matrix &m)
{
    if (height() != m.height())
        return Matrix();

    Matrix ret(width()+m.width(), height());
    for (int iy = 0; iy < height(); ++iy)
    {
        int ix = 0;
        while (ix < ret.width())
        {
            if (ix < width())
                ret(ix,iy, mat(ix,iy));
            else
                ret(ix,iy, m(ix,iy));
            ++ix;
        }
    }

    return ret;
}

Matrix Matrix::appendcol(Matrix &m)
{
    if (width() != m.width())
        return Matrix();

    Matrix ret(width(), height()+m.height());
    int iy = 0;
    while (iy < ret.height())
    {
        if (iy < height())
            for (int ix = 0; ix < width(); ++ix)
                ret(ix,iy, mat(ix,iy));
        else
            for (int ix = 0; ix < width(); ++ix)
                ret(ix,iy, m(ix,iy));

        ++iy;
    }

    return ret;
}

bool  Matrix::SVD(Matrix &e, Matrix &E)
{
    Matrix S(this);
    S.makeSq();
    S *= S.T();
    int state, n = S.width();
    Matrix ind = Matrix(S.width(),1);
    Matrix changed= Matrix(S.width(),1);

    E = eye(n);
    e = Matrix(n,1);
    for (int k = 0; k < n; ++k)
    {
        ind(k,1, maxind(k,S));
        e(k,1, S(k,k));
        changed(k,1, 1);
    }

    int m, l, p;
    double y, d, r, c, s, t;
    while (state > 0)
    {
        m = 0;
        for (int k = 1; k < n-1; ++k)
            if(abs(S(k,ind(k,1))) > abs(S(m,ind(m,1))))
                m = k;
        l = ind(m,1); p = S(m,l);
        y = (e(1,1)-e(k,1))/2; d = abs(y)+ sqrt(p*p + y*y);
        r = sqrt(p*p + d*d); c = d/r; s = p/r; t = p*p/d;
        if (y < 0)
        {
            s = -s; t = -t;
        }
        S(m,l,0);
        y = e(m,1); e(m,i,y-t);
        if (changed(m,1) && y == e(m,1))
        {
            changed(m,1,0);
            --state;
        }
        else if (!changed(m,1) && y != e(m,1))
        {
            changed(m,1,1);
            ++state;
        }
        y = e(l,1); e(l,i,y+t);
        if (changed(l,1) && y == e(l,1))
        {
            changed(l,1,0);
            --state;
        }
        else if (!changed(l,1) && y != e(l,1))
        {
            changed(l,1,1);
            ++state;
        }

        for (int k = 0; k < m-1; ++k)
            rotate(S,k,m,k,l,c,s);
        for (int k = m; k < l-1; ++k)
            rotate(S,m,k,k,l,c,s);
        for (int k = l; k < n; ++k)
            rotate(S,m,k,l,k,c,s);
        for (int k = 0; k < n; ++k)
            rotate(E,k,m,k,l,c,s);

        ind(m,1, maxind(m,S)); ind(l,1, maxind(l,S));
    }
}

int Matrix::maxind(int k, const Matrix &m)
{
    int ret = k+1;
    for (int i = k+2; i < m.width(); ++i)
        if (abs(m(k,i)) > abs(m(k,ret)))
            ret = i;
    return ret;
}

void Matrix::rotate(Matrix &m, int k, l, i, j, c, s)
{
    m(k,l, c*m(k,l) - s*(m(i,j)));
    m(i,j, s*m(k,l) + c*(m(i,j)));
}
