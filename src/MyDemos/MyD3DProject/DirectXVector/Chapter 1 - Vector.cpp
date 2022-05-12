// DirectXVector.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

void XM_CALLCONV XmvectorParameterExampleFunction(
					FXMVECTOR xmvec1,
					FXMVECTOR xmvec2,
					FXMVECTOR xmvec3,
					GXMVECTOR xmvec4,
					HXMVECTOR xmvec5,
					HXMVECTOR xmvec6,
					CXMVECTOR xmvec7
				)
{
	//do smt
}

int main()
{
	XMVECTOR xmvec1{1.0f,2.0f,3.0f,1.0f };
	XMVECTOR xmvec2{4.0f,5.0f,6.0f,1.0f };
	XMVECTOR xmvec3{7.0f,8.0f,9.0f,1.0f };
	XMVECTOR xmvec4{10.0f,11.0f,12.0f,1.0f };
    cout << "xmvec1: " << xmvec1 << endl;
    cout << "xmvec2: " << xmvec2 << endl;
    cout << "xmvec3: " << xmvec3 << endl;
    cout << "xmvec4: " << xmvec4 << endl;

 //   xmvec1 = XMVectorSet(5.0f, 6.0f, 7.0f, 8.0f); // set XMVECTOR using XMVectorSet
 //   cout << xmvec1 << endl;

 //   xmvec1 = XMVectorSetX(xmvec1, 10.0f); //set XMVECTOR using XMVectorSetX/Y/Z
 //   cout << xmvec1 << endl;

	//XMFLOAT2 fVec2D(3.0f, 4.0f);
 //   xmvec1 = XMLoadFloat2(&fVec2D); //set XMVECTOR using XMLoadFloat2/3/4
 //   cout << xmvec1 << endl;

	//const XMVECTORF32 constantVec {0.5f,0.5f, 0.5f, 0.5f}; //constant should be of type XMVECTORF32 or XMVECTORU32/XMVECTORI32
	//cout << constantVec.v << endl;

	cout << "xmvec1 + xmvec2 = " << xmvec1 + xmvec2 << endl;
	cout << "xmvec3 * xmvec4 = " << xmvec3 * xmvec4 << endl;
	cout << "PI to degree: " << XMConvertToDegrees(XM_PI) << endl;
	cout << "Length of xmvec1 = " << XMVector3Length(xmvec1) << endl;
	cout << "Normalize xmvec2 = " << XMVector3Normalize(xmvec2) << endl;
	cout << "Dot product xmvec1.xmvec2 = " << XMVector3Dot(xmvec1, xmvec2) << endl;
	cout << "Cross product xmvec1 X xmvec2 = " << XMVector3Cross(xmvec1, xmvec2) << endl;
	cout << "A vector v orthogonal to xmvec1; v = " << XMVector3Orthogonal(xmvec1) << endl;
	cout << "Angle between xmvec1 and xmvec2 is (radiant) " << XMVector3AngleBetweenVectors(xmvec1, xmvec2) << endl;
	cout << "Length of xmvec1 (estimate) = " << XMVector3LengthEst(xmvec1) << endl;
	cout << "Normalize xmvec2 (estimate)= " << XMVector3NormalizeEst(xmvec2) << endl;

	/*floating-point precision*/
	float length = XMVectorGetX(XMVector3Length(XMVector3Normalize(xmvec1)));
	cout.precision(8);
	cout << length << endl;
	cout << "length^10^6 = " << pow(length, 1.0e6f) << endl;
	XMVECTOR lengthPowVec = XMVectorReplicate(pow(length, 1.0e6f));
	XMVECTOR epsilonVec = XMVectorReplicate(0.000001f);
	cout << "is near equal with epsilon = 0.000001f : " << (XMVector3NearEqual(XMVector3Length(XMVector3Normalize(xmvec1)), lengthPowVec, epsilonVec)? "true" : "false") << endl;
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
