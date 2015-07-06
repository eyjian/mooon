#!/bin/sh
# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

format_function="CStringUtils::format_string"
if test $# -eq 0; then
	paramter_number=5
else
	paramter_number=$1
fi
	
echo "// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)"
echo "#ifndef MOOON_UTILS_FORMAT_STRING_H"
echo "#define MOOON_UTILS_FORMAT_STRING_H"
echo "#include <mooon/utils/string_utils.h>"
echo "#include <string>"
echo "#include <vector>"
echo "UTILS_NAMESPACE_BEGIN"
echo ""
echo "enum { FORMAT_STRING_SIZE = $paramter_number };"
echo "inline std::string format_string(const char* format, const std::vector<std::string>& tokens)"
echo "{"

for ((i=0; i<$paramter_number; ++i))
do
	echo "    if ($i == tokens.size())"
	echo "    {"

	str="$format_function(format"
	for ((j=0; j<$i; ++j))
	do
		str=${str}", tokens[$j].c_str()"
	done
	str=${str}");"
	echo "        return $str"
	echo "    }"
done
echo "    return std::string(\"\");"
echo "}"
echo ""
echo "UTILS_NAMESPACE_END"
echo "#endif // MOOON_UTILS_FORMAT_STRING_H"
echo ""

exit 0

