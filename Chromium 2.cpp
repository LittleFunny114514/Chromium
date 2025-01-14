// Chromium 2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
import std;
import Chromium;
import Chromium.NN.LA;
Chromium::Int<false, false> n1 = 114514, n2 = 1919810;
#define time(expression,ret) { auto before = std::chrono::high_resolution_clock::now(); expression; \
	ret = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - before).count(); }
//*
int main()
{

	using namespace Chromium;
	using std::cout, std::endl;
	//test();
	//*
	size_t N = 4096;
	Matrix<float>A(N), B(N), C(N), D(N);
	double t;
	time(A * B, t);
	cout <<std::setprecision(15)<< t / 1E9;//*/
	/*
	Matrix<float>A(1024, 1024, 0), B(1024, 1024, 0), C;
	for (float& num : A.dat)num = float(MathHelper::randUint()) / 0xFFFF;
	for (float& num : B.dat)num = float(MathHelper::randUint()) / 0xFFFF;
	long long t; time(A*B; , t)
	cout << t/1E9;//*/
	//std::cout << (n1*n2).hex();
	/*Matrix<float>A(2, 3, 0), B(4, 2, 0), C;
	int ni = 0;
	for (float& num : A.dat)num = ++ni;
	ni = 8;
	for (float& num : B.dat)num = ni--;
	cout << A.dat.toString() << endl << B.dat.toString() << endl << (A * B).dat.toString();*/
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
