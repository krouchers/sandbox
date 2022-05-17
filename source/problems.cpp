#include <problems.h>
#include <string>
#include <application.h>

int answers[5] = {
    116, 2, 50, 36, 36};

problems::problems(application *app) : _app{app},
                                       _problems_count{0}
{
    char buf[8];
    for (size_t i = 1;; ++i)
    {
        stream *problem_text = _app->get_file_system().open("misc/problems/" + std::string(itoa(i, buf, 10)) + ".txt");
        if (!problem_text)
            break;

        _problems.push_back(problem("../assets/models/" + std::string(_problems_names[i - 1]) + ".obj", problem_text->read(), answers[i - 1]));
        _app->get_file_system().close(problem_text);
    }
}

problem &problems::operator[](problems_list problem)
{
    return _problems[static_cast<size_t>(problem)];
}