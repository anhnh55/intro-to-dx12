// Chapter2 - Matrix.cpp : Demo Matrix creation and manipulation in DirectX
//

#include <iostream>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using namespace DirectX;
using namespace PackedVector;
using namespace std;

ostream& XM_CALLCONV operator<<(ostream& os, FXMVECTOR v)
{
    XMFLOAT4 dest;
    XMStoreFloat4(&dest, v);

    os << "(" << dest.x << ", " << dest.y << ", " << dest.z << ", " << dest.w << ")";
    return os;
}

ostream& XM_CALLCONV operator<<(ostream& os, FXMMATRIX m)
{
    for (int i = 0; i < 4; ++i)
    {
        os << XMVectorGetX(m.r[i]) << "\t";
        os << XMVectorGetY(m.r[i]) << "\t";
        os << XMVectorGetZ(m.r[i]) << "\t";
        os << XMVectorGetW(m.r[i]);
        os << endl;
    }
    return os;
}

int main()
{
    XMMATRIX M1(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 4.0f, 0.0f,
        1.0f, 2.0f, 3.0f, 1.0f);

   /* XMMatrixSet(1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 4.0f, 0.0f,
        1.0f, 2.0f, 3.0f, 1.0f);*/

    cout << "Matrix M1: " << endl << M1;

    XMVECTOR r1 = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
    XMVECTOR r2 = XMVectorSet(2.0f, 2.0f, 2.0f, 2.0f);
    XMVECTOR r3 = XMVectorSet(3.0f, 3.0f, 3.0f, 3.0f);
    XMVECTOR r4 = XMVectorSet(4.0f, 4.0f, 4.0f, 4.0f);

    XMMATRIX M2(r1,r2,r3,r4);
    cout << "Matrix M2: " << endl << M2;
   
    cout << "Transpose of M1: " << endl << XMMatrixTranspose(M1);

    XMVECTOR det = XMMatrixDeterminant(M1);
    cout << "Determinant of M1: " << det << endl;

    XMMATRIX inverseM1 = XMMatrixInverse(&det, M1);
    cout << "Inverse of M1: " << endl << inverseM1;

    cout << "M1 * inverse of M1: " << endl << M1 * inverseM1;
    cout << "M1 + M2: " << endl << M1 + M2;

    XMMATRIX Identity = XMMatrixIdentity();

    cout << "Identity Matrix: " << endl << Identity;

    XMFLOAT4X4 float4x4;
    XMStoreFloat4x4(&float4x4, M1);
    float4x4.m[0][0] = 100.0f;
    M1 = XMLoadFloat4x4(&float4x4);
    cout << "M1 after loading: " << endl << M1;
    
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
