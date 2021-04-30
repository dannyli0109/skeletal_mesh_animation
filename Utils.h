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

static void SaveImage(std::string path, GLFWwindow* window)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    GLsizei nrChannels = 3;
    GLsizei stride = nrChannels * width;
    GLsizei bufferSize = stride * height;
    std::vector<char> buffer(bufferSize);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
    stbi_flip_vertically_on_write(true);
    stbi_write_png(path.c_str(), width, height, nrChannels, buffer.data(), stride);
}
