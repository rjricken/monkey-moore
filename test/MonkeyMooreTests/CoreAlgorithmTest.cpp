#include <array>
#include <memory>
#include <fstream>
#include "CppUnitTest.h"

#include <wx/filefn.h> 

#include "..\..\src\monkey_moore.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace MonkeyMooreTests
{
   /**
    * Set of tests to check the correctness of the core algorithm in Monkey-Moore.
    * These tests include the following variations of settings:
    * - Simple, Wildcard and Value Scan
    * - 8 and 16-bit
    * - ASCII, Unicode (Japanese) and Custom sets
    * - Single and multiple results
    */
	TEST_CLASS(CoreAlgorithmTest)
	{
   private:
      /**
       * Checks the correctness of a single result match in a Monkey-Moore search.
       * @tparam _Type The underlying data type used by Monkey-Moore to interpret data
       * @param result The result obtained from performing a search
       * @param expected The expected results from that search
       */
      template <typename _Type> void checkResultMatch(
         const std::pair<long, std::map<wxChar, _Type>>& result,
         const std::pair<long, std::map<wxChar, _Type>>& expected)
      {
         // Checks whether the result match has the expected address
         Assert::AreEqual<long>(result.first, expected.first,
            wxT("Failed to return the correct address of a result match"));

         // Checks whether both maps in the result match have the same number of elements
         Assert::AreEqual<size_t>(expected.second.size(), result.second.size(),
            wxT("Failed to return the correct number of values in a result match"));

         for (const auto& kv : expected.second)
         {
            // Checks whether the current element in the map of expected values exists in the result
            Assert::AreEqual<size_t>(1, result.second.count(kv.first),
               wxT("Failed to return the correct values of a result match"));

            // Checks whether the corresponding value of the current element in the
            // map of expected values matches the value in the result
            Assert::AreEqual<_Type>(kv.second, result.second.at(kv.first),
               wxT("Failed to return the correct values of a result match"));
         }
      }

      /**
       * Checks the correctness of a result set in a Monkey-Moore search.
       * @tparam _Type The underlying data type used by Monkey-Moore to interpret data
       * @param result The result set obtained from performing a search
       * @param expected The expected result set from that search
       */
      template <typename _Type> void checkSearchResults(
         const std::vector<std::pair<long, std::map<wxChar, _Type>>>& results,
         const std::vector<std::pair<long, std::map<wxChar, _Type>>>& expected)
      {
         // Checks whether the result contains the expected number of matches
         Assert::AreEqual<size_t>(expected.size(), results.size(),
            wxT("Failed to return correct number of results"));

         for (auto i = 0; i < results.size(); ++i)
            checkResultMatch<_Type>(results[i], expected[i]);
      }

      /**
       * Creates a Monkey-Moore result match from the specified parameters (ASCII only).
       * The result match is an instance of std::pair<long, std::map<wxChar, _Type>>.
       * @tparam _Type The underlying data type used by Monkey-Moore to interpret data
       * @param offset The address where this match was found at
       * @param value_A The found value of 'A'
       * @param value_a The found value of 'a'
       * @return A pair containing the offset and matching values
       */
      template <typename _Type>
      std::pair<long, std::map<wxChar, _Type>> createMatchAscii(long offset, _Type value_A, _Type value_a)
      {
         std::map<wxChar, _Type> matches;
         matches.insert(std::make_pair('A', value_A));
         matches.insert(std::make_pair('a', value_a));

         return std::make_pair(offset, matches);
      }

      template <typename _DataType>
      std::pair<size_t, std::shared_ptr<_DataType>> readDataFromFile(const std::string &fileName)
      {
         std::ifstream fileHandle(fileName, std::ios::binary | std::ios::ate);
         Assert::IsFalse(!fileHandle == true, wxString::Format(wxT("Failed to open file needed in test: %s"), fileName).c_str());

         auto fileSize = fileHandle.tellg();
         fileHandle.seekg(std::ios::beg);

         std::shared_ptr<_DataType> data(new _DataType[fileSize], std::default_delete<_DataType[]>());
         fileHandle.read(reinterpret_cast<char*>(data.get()), fileSize);

         return std::make_pair(fileSize, data);
      }

	public:
      /**
       * Test for a basic search using 8-bit data, on ASCII mode, with a single result.
       */
		TEST_METHOD(Basic_8bit_ASCII_SingleResult)
		{
         const wxChar wildcard = wxT('');
         const wxString keyword = "incredible";

         // Matches:
         // 16 - 'a': 0x69, 'A': 0x49
         std::string data = "pqa pia jmmv iv qvkzmlqjtm rwczvmg, jcb qb pia kwum bw iv mvl.";
         char *dataPtr = const_cast<char*>(data.data());

         MonkeyMoore<uint8_t> moore(keyword, wildcard);
         auto results = moore.search(reinterpret_cast<uint8_t*>(dataPtr), data.length());

         // expected result
         std::vector<MonkeyMoore<uint8_t>::relative_type> expected;
         expected.push_back(createMatchAscii<uint8_t>(16, 0x49, 0x69));

         checkSearchResults<uint8_t>(results, expected);
		}

      /**
       * Test for a basic search using 8-bit data, on ASCII mode,
       * with multiple results (and different values for each one)
       */
      TEST_METHOD(Basic_8bit_ASCII_MultipleResults)
      {
         const wxChar wildcard = wxT('');
         const wxString keyword = "grotesque";

         // Matches:
         // 11 - 'a': 0x64, 'A': 0x44
         // 32 - 'a': 0x66, 'A': 0x46
         // 64 - 'a': 0x63, 'A': 0x43
         std::string data = "wklv lv dq jurwhvtxh gdb, lq dq lwtyjxvzj zhhn, lq dq hyhq pruh itqvguswg bhdu.";
         char *dataPtr = const_cast<char*>(data.data());

         MonkeyMoore<uint8_t> moore(keyword, wildcard);
         auto results = moore.search(reinterpret_cast<uint8_t*>(dataPtr), data.length());

         // expected result
         std::vector<MonkeyMoore<uint8_t>::relative_type> expected;
         expected.push_back(createMatchAscii<uint8_t>(11, 0x44, 0x64));
         expected.push_back(createMatchAscii<uint8_t>(32, 0x46, 0x66));
         expected.push_back(createMatchAscii<uint8_t>(64, 0x43, 0x63));

         checkSearchResults<uint8_t>(results, expected);
      }

      /**
       * Test for a basic search using 8-bit data, on ASCII mode, with no results.
       */
      TEST_METHOD(Basic_8bit_ASCII_NoResults)
      {
         const wxChar wildcard = wxT('');
         const wxString keyword = "brotesque";

         // Matches: none
         std::string data = "wklv lv dq jurwhvtxh gdb, lq dq lwtyjxvzj zhhn, lq dq hyhq pruh itqvguswg bhdu.";
         char *dataPtr = const_cast<char*>(data.data());

         MonkeyMoore<uint8_t> moore(keyword, wildcard);
         auto results = moore.search(reinterpret_cast<uint8_t*>(dataPtr), data.length());

         // expected result
         std::vector<MonkeyMoore<uint8_t>::relative_type> expected;

         checkSearchResults<uint8_t>(results, expected);
      }

      TEST_METHOD(Wildcard_8bit_ASCII_MultipleResults)
      {
         const wxChar wildcard = wxT('*');
         const wxString keyword = "Spira*";

         //const std::array<std::string, 5> keywords = { "Spira*", "*ver", "Every**e", "Never", "h**e" };
         /*const int offsets[][] = { { 120, 306 },
                                   { 0, 38 },
                                   { 0, 38 },
                                   { 37, 467},
                                   { 47, 61, 179, 261, 423, 453 } };*/

         // Matches:
         // 120 - 'a': 0x6D, 'A': 0x4D
         // 306 - 'a': 0x6D, 'A': 0x4D
         // __________________________
         // Everyone has lost something precious. Everyone here has lost homes, dreams, and friends. Now, Sin is finally dead. Now, Spira 
         // is ours again. Working together, now we can make new homes for ourselves, and new dreams. Although I know the journey will be
         // hard, we have lots of time. Together, we will rebuild Spira. The road is ahead of us, so let's start out today. Just, one more
         // thing... the people and the friends that we have lost, or the dreams that have faded... Never forget them.
         auto data = readDataFromFile<uint8_t>("Wildcard_8bit_ASCII_MultipleResults_Enc.bin");
         uint8_t* dataPtr = data.second.get();

         MonkeyMoore<uint8_t> moore(keyword, wildcard);
         auto results = moore.search(dataPtr, data.first);

         std::vector<MonkeyMoore<uint8_t>::relative_type> expected;
         expected.push_back(createMatchAscii<uint8_t>(120, 0x4D, 0x6D));
         expected.push_back(createMatchAscii<uint8_t>(306, 0x4D, 0x6D));

         checkSearchResults<uint8_t>(results, expected);
      }
	};
}