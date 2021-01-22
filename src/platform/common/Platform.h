#ifndef PLATFORM_H_
#define PLATFORM_H_

class App;

/**
 * Defines a platform abstraction.
 *
 * This class has only a few public methods for creating a platform
 */
class Platform
{
public:


    /**
     * Destructor.
     */
    ~Platform();

    /**
     * Creates a platform for the specified game which it will interact with.
     *
     * @param app The app to create a platform for.
     *
     * @return The created platform interface.
     * @script{ignore}
     */
    static Platform* create(App* app);

    /**
     * Begins processing the platform messages.
     *
     * This method handles all OS window messages and drives the game loop.
     * It normally does not return until the application is closed.
     *
     * If a attachToWindow is passed to Platform::create the message pump will instead attach
     * to or allow the attachToWindow to drive the game loop on the platform.
     *
     * @return The platform message pump return code.
     */
    int enter_message_pump();

};

#endif
