#include "Helper.h"

#pragma region STRING_RELATED_HELPERS

bool Help_IsContainPattern(const std::string& src, const std::string& pattern)
{
	unsigned int patternIndex = 0;
	unsigned int srcIndex = 0;

	bool metAll = false;

	while (srcIndex < src.size())
	{
		if (patternIndex >= pattern.size()) break; // break if all the pattern index exceeded.
		
		if (src[srcIndex] == pattern[patternIndex])
		{
			patternIndex++;
		}else
		{
			// Reset if pattern discontinued
			patternIndex = 0;
		}

		if (patternIndex == pattern.size())
		{
			// met all chars.
			metAll = true;
			break;
		}

		srcIndex++;
	}

	return metAll;
}

bool Help_IsSrcExtensionOf(const std::string& src, const std::string& pattern)
{
	if (pattern.empty() || src.empty()) return false;

	std::string refinedPattern;

	if (pattern[0] != '.')
	{
		refinedPattern = '.' + pattern;
	}
	else refinedPattern = pattern;

 	bool result = false;
	int rightOfPattern = static_cast<int>(refinedPattern.size()) - 1;
	int rightOfSrc = static_cast<int>(src.size()) - 1;

	while (rightOfPattern >= 0 && rightOfSrc >= 0)
	{

		if (refinedPattern[rightOfPattern] != src[rightOfSrc])
		{
			result = false;
			break;
		}

		rightOfPattern--;
		rightOfSrc--;

		if (rightOfPattern == -1)
		{
			result = true;
			break;
		}
	}

	return result;
}

#pragma endregion