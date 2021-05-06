//These includes are specific to the way we’ve set up GLFW and GLAD.
#include "ProgramManager.h"
int main(void)
{
    ProgramManager programManager;
    programManager.Init();
    programManager.Update();
    programManager.Destroy();
    return 0;
}
