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
