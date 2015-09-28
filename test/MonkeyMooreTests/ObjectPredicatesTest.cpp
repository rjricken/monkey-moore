#include <array>
#include <algorithm>
#include "CppUnitTest.h"

#include "..\..\src\object_pred.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MonkeyMooreTests
{		
	TEST_CLASS(ObjectPredicatesTest)
	{
	public:
		
		TEST_METHOD(FindLast)
		{
         std::array<int, 10> data = {3, 3, 5, 7, 6, 3, 8, 9, 3, 10};

         int lastPos = find_last(data.begin(), data.end(), 3);
         int notFound = find_last(data.begin(), data.end(), 2);

         Assert::AreEqual<int>(8, lastPos);
         Assert::AreEqual<int>(0, notFound);
		}

      TEST_METHOD(CountBegin)
      {
         std::array<int, 10> data = {3, 3, 3, 3, 6, 3, 8, 9, 3, 10};

         int countNum3 = count_begin(data.begin(), data.end(), 3);
         int countNum6 = count_begin(data.begin(), data.end(), 6);
         int countNum2 = count_begin(data.begin(), data.end(), 2);

         Assert::AreEqual<int>(4, countNum3);
         Assert::AreEqual<int>(0, countNum6);
         Assert::AreEqual<int>(0, countNum2);
      }

      TEST_METHOD(IsUpper_IsLower)
      {
         std::string upperLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
         std::string lowerLetters = "abcdefghijklmnopqrstuvwxyz";
         std::string mixed = "aBCdefGHiJkLMNOPqrStuvwxYz";

         auto count = std::count_if(upperLetters.begin(), upperLetters.end(), is_upper);
         Assert::AreEqual<int>(26, count, wxT("is_upper failed with upperLetters"));

         count = std::count_if(lowerLetters.begin(), lowerLetters.end(), is_upper);
         Assert::AreEqual<int>(0, count, wxT("is_upper failed with lowerLetters"));

         count = std::count_if(mixed.begin(), mixed.end(), is_upper);
         Assert::AreEqual<int>(12, count);
      }

	};
}