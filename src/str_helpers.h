#include <string>
#include <sstream>
#include <vector>

namespace str_helpers
{
    /*
        Заменяет все найденные строки match в строке data на строку replace.

        data - строка в которой нужно искать
        match - строка которую нужно заменить
        replace - строка на которую нужно заменить
    */
    std::string replace_all(std::string data, const std::string match, const std::string replace) 
    {
        std::string final_string;

        while(true)
        {
            std::ostringstream tmp_string;
            size_t pos = data.find(match);

            if (pos == std::string::npos)
                break;

            for (int i = 0; i < pos; i++)
            {
                tmp_string << data[i];
            }

            tmp_string << replace;

            for (int i = pos + match.length(); i < data.length(); i++)
            {
                tmp_string << data[i];
            }

            data = tmp_string.str();
            pos = data.find(match);
        }

        if (final_string.length() == 0)
            return data;
        else
            return final_string;
    }

    /*
        Делит строку на массив из строк на основе разделителя match
        data - оригинальная строка
        match - разделитель
    */
    std::vector<std::string> split(std::string data, const std::string match)
    {
        std::vector<size_t> found_positions = {};
        std::vector<std::string> final_vector = {};

        size_t current_position = 0;
        while (true)
        {
            size_t result = data.find(match, current_position);

            if (result == std::string::npos)
                break;

            found_positions.push_back(result);
            current_position = result + 1;
        }

        if (found_positions.size() != 0)
        {
            int last_index = 0;
            for (int i = 0; i < found_positions.size() + 1; i++)
            {
                std::string match_string = "";
                std::ostringstream string_stream;

                int end_index = found_positions[i];
                if (i == found_positions.size())
                    end_index = data.length();

                for (int k = last_index; k < end_index; k++)
                {
                    string_stream << data[k];
                }

                last_index = found_positions[i] + match.length();
                match_string = string_stream.str();
                final_vector.push_back(match_string);
            }
            return final_vector;
        }

        return {data};
    }

    /*
        Проверяет что строка data начинается со строки match
        data - оригинальная строка
        match - строка с которой надо сравнить начало
    */
    bool starts_with(const std::string data, const std::string match)
    {
        if (data.length() < match.length())
            return false;

        for (int i = 0; i < match.length(); i++)
        {
            if (data[i] != match[i])
                return false;
        }

        return true;
    }
}