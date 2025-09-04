#include "../includes/Helper.hpp"

// Check if password contains only alphanumeric characters
bool Helper::isPasswordValid(std::string pass)
{
	for (size_t i = 0; i < pass.length(); i++)
	{
		if (!isalnum(pass.at(i)))
			return false;
	}
	return true;
}