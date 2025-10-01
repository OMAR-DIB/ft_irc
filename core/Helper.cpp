#include "../includes/Helper.hpp"

bool Helper::isPasswordValid(std::string pass)
{
	for (size_t i = 0; i < pass.length(); i++)
	{
		if (!isalnum(pass.at(i)))
			return false;
	}
	return true;
}