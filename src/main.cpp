void serialEvent();

#include "application.hpp"

Application *app; 

void setup()
{
    app = new Application();
    app->initialize();
}
void loop()
{
    app->mainloop();
}