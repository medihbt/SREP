#include <stdlib.h>
#include <stdio.h>

typedef struct __mt
{
    int height;
    int width;
    double *value;
} Matrix;

/*二维动态数组难以实现, 转而用一维数组模拟二维数组*/
#define Matrix_index(M, i, j) ((M).value[(i) * (M).width + (j)]) // M[i][j]
#define Matrix_row(M, i) (&((M).value[(i) * (M).width]))         // M[i]

/*构造函数*/
Matrix new_matrix(int height, int width)
{
    Matrix matrix = {height, width, NULL};
    matrix.value = (double *)malloc(width * height * sizeof(double));
    return matrix;
}
/*Matrix.input()输入*/
void matrix_input(Matrix m)
{
    printf("input your matrix (%d lines, %d cols) : \n", m.height, m.width);
    for (int i = 0; i < m.height; i++)
        for (int j = 0; j < m.width; j++)
            scanf("%lf", &Matrix_index(m, i, j));
}

void matrix_print(Matrix m)
{
    if (m.value == NULL)
        return;
    for (int i = 0; i < m.height; i++)
    {
        for (int j = 0; j < m.width; j++)
            printf("%lf\t", Matrix_index(m, i, j));
        putchar('\n');
    }
}

/* Function: return a*b */
Matrix matrix_multiply(Matrix a, Matrix b)
{
    Matrix ret = {0, 0, NULL};
    if (a.width != b.height)
        return ret;

    ret = new_matrix(b.width, a.height);
    for (int i = 0; i < a.height; i++)
    {
        for (int j = 0; j < b.width; j++)
        {
            Matrix_index(ret, i, j) = 0.0;
            for (int k = 0; k < a.width; k++)
                Matrix_index(ret, i, j) += Matrix_index(a, i, k) * Matrix_index(b, k, j);
        }
    }
    return ret;
}

int main()
{
    Matrix a = new_matrix(4, 1);
    Matrix b = new_matrix(1, 4);
    matrix_input(a);
    matrix_input(b);
    Matrix c = matrix_multiply(a, b);
    matrix_print(c);
    return 0;
}
