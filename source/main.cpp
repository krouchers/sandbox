#include <application.h>

int main(int argc, char **argv)
{
    (void)argc;
    application app{
        1280, 720, "MyApplication", argv[0]};
    app.run();
}
