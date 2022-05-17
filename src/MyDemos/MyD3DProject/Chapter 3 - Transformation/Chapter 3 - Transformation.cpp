// Chapter 3 - Transformation.cpp : This file contains the 'main' function. Program execution begins and ends there.
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
    XMVECTOR vector{ 1.0f,2.0f,3.0f,1.0f };
    XMMATRIX scalingMatrix = XMMatrixScaling(1.0f, 2.0f, 3.0f);
    XMMATRIX rotateXMatrix = XMMatrixRotationX(45.0f);
    XMMATRIX translateMatrix = XMMatrixTranslation(1.0f, 2.0f, 3.0f);
    XMMATRIX SRTMatrix = scalingMatrix * rotateXMatrix * translateMatrix;
    cout << "Vector (or Point): " << vector << endl;
    cout << "Scaling Matrix: \n" << scalingMatrix;
    cout << "Rotation Matrix (X axis): \n" << rotateXMatrix;
    cout << "Translate Matrix: \n" << translateMatrix;
    cout << "SRT Matrix: \n" << SRTMatrix;
    XMVECTOR scaledVector = XMVector3TransformCoord(vector, scalingMatrix);
    XMVECTOR rotatedVector = XMVector3TransformCoord(scaledVector, rotateXMatrix);
    XMVECTOR translatedVector = XMVector3TransformCoord(rotatedVector, translateMatrix);
    cout << "Scale: \n" << scaledVector << endl;
    cout << "Then Rotate: \n" << rotatedVector << endl;
    cout << "Then Translate: \n" << translatedVector << endl;
    cout << "Transform using SRT Matrix: \n" << XMVector3TransformCoord(vector, SRTMatrix) << endl;
    cout << "Ignore Translation: \n" << XMVector3TransformNormal(vector, SRTMatrix) << endl;
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
