static std::string LoadFileAsString(std::string filename)
{
    std::stringstream fileSoFar;
    std::ifstream file(filename);

    if (file.is_open())
    {
        while (!file.eof())
        {
            std::string line;
            std::getline(file, line);
            fileSoFar << line << std::endl;
        }
        return fileSoFar.str();
    }
    else
    {
        return "";
    }
}


static bool VectorOfStringGetter(void* vec, int idx, const char** out_text)
{
    auto& vector = *static_cast<std::vector<std::string>*>(vec);
    if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
    *out_text = vector.at(idx).c_str();
    return true;
}

template<typename T, typename N>
static std::vector<N> MapArray(std::vector<T>& array, N (*f)(T val))
{
    std::vector<N> output;
    for (int i = 0; i < array.size(); i++)
    {
        output.push_back(f(array[i]));
    }
    return output;
}