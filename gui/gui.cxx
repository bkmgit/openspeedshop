#include <stdio.h> 

#include <qapplication.h>
#include "openspeedshop.hxx"
#include <qvaluelist.h>

#include "PanelContainer.hxx"

QApplication *qapplication;

#include <qeventloop.h>
#include <qlineedit.h>
QEventLoop *qeventloop;

#include "splash.xpm"
#include <qsplashscreen.h>
#include <qcheckbox.h>

#include <pthread.h>
#define PTMAX 10
pthread_t phandle[PTMAX];
#include "ArgClass.hxx"


extern "C" 
{
  // This routine starts another QApplication gui.  It is called from 
  // gui_init to have the gui started in it's own thread.
  void
  guithreadinit(void *ptr)
  {
    ArgStruct *arg_struct = NULL;
    int argc = 0;
    char **argv = NULL;
    if( ptr != NULL )
    {
      arg_struct = (ArgStruct *)ptr;
      argc = arg_struct->argc;
      argv = arg_struct->argv;
    }
    bool splashFLAG=TRUE;
  
// printf("guithreadinit() \n");
    QString fontname = QString::null;
    for(int i=0;i<argc;i++)
    {
// printf("  argv[%d]=(%s)\n", i, argv[i] );
      QString arg = argv[i];
      if( arg == "-fn" || arg == "--fn" )
      {
        fontname = argv[i+1];
        break;
      }
    }

    qapplication = new QApplication( argc, argv );

    if( argv == NULL )
    {
      argc = 0;
    }

    QString hostStr = QString::null;
    QString executableStr = QString::null;
    QString widStr = QString::null;
    QString argsStr = QString::null;
    QString pidStr = QString::null;
    QString rankStr = QString::null;
    QString expStr = QString::null;
    for(int i=0;i<argc;i++)
    {
      QString arg = argv[i];
#ifdef OLDWAY
      if( arg == "--no_splash" ||
          arg == "-ns" ) 
      {
        splashFLAG = FALSE;
      } else if( arg == "-f" )
      {  // Get the target executableName.
        executableStr = QString(argv[++i]);
      } else if( arg == "-h" )
      { // Get the target host (or host list)
        hostStr = QString(argv[++i]);
      } else if( arg == "-r" )
      { // attach to the rank (list) specified
        rankStr = QString(argv[++i]);
      } else if( arg == "-p" )
      { // attach to the proces (list) specified )
        pidStr = QString(argv[++i]);
      } else if( arg == "-x" )
      { // load the collector (experiment) 
        expStr = QString(argv[++i]);
      } else if( arg == "-wid" )
      { // You have a window id from the cli
        for( ;i<argc;)
        {
          widStr += QString(argv[++i]);
          widStr += QString(" ");
        }
      } else if( arg == "-a" )
      { // load the command line arguments (to the exectuable)
        for( ;i<argc;)
        {
          argsStr += QString(argv[++i]);
          argsStr += QString(" ");
        }
      } else if( arg == "-help" || arg == "--help" )
      {
//        usage();
        return; // Failure to complete...
      } else if( arg == "-cli" )
      {
        // Valid: though not currently documented.   Just ignore 
        // and fall through.
      } else if( !arg.contains("openspeedshop") )
      {
        printf("Unknown argument syntax: argument in question: (%s)\n", arg.ascii() );
//        usage();
        return; // Failure to complete...
      }
#else // OLDWAY
      if( arg == "-wid" )
      { // You have a window id from the cli
        for( ;i<argc;)
        {
          widStr += QString(argv[++i]);
          widStr += QString(" ");
        }
      }
#endif // OLDWAY

    }

    OpenSpeedshop *w;

    w = new OpenSpeedshop();

    QPixmap *splash_pixmap = NULL;
    QSplashScreen *splash = NULL;
    if( !w->preferencesDialog->setShowSplashScreenCheckBox->isChecked() )
    {
      splashFLAG = FALSE;
    } 
    if( splashFLAG )
    {
      splash_pixmap = new QPixmap( splash_xpm );
      splash = new QSplashScreen( *splash_pixmap );
      splash->setCaption("splash");
      splash->show();
      splash->message( "Loading plugins" );
      splash->raise();
    }

    // Set the font from the preferences...
    QFont *m_font = NULL;
    if( w->preferencesDialog->preferencesAvailable == TRUE )
    {
      m_font = new QFont( w->preferencesDialog->globalFontFamily,
                      w->preferencesDialog->globalFontPointSize,
                      w->preferencesDialog->globalFontWeight,
                      w->preferencesDialog->globalFontItalic );
    }

    if( m_font != NULL )
    {
      qApp->setFont(*m_font, TRUE);

      delete m_font;
    }


    w->executableName = executableStr;
    w->widStr = widStr;
    w->pidStr = pidStr;
    w->rankStr = rankStr;
    w->expStr = expStr;
    w->hostStr = hostStr;
    w->argsStr = argsStr;

#ifdef OLDWAY
    if( w->expStr != NULL )
    {
      if( w->expStr == "pcsamp" )
      {
        w->topPC->dl_create_and_add_panel("pc Sampling");
      } else if( w->expStr == "usertime" )
      {
        w->topPC->dl_create_and_add_panel("User Time");
      } else if( w->expStr == "fpe" )
      {
        w->topPC->dl_create_and_add_panel("FPE Tracing");
      } else if( w->expStr == "hwc" )
      {
        w->topPC->dl_create_and_add_panel("HW Counter");
      } else if( w->expStr == "io" )
      { 
        w->topPC->dl_create_and_add_panel("IO");
      } else if( w->expStr == "mpi" )
      { 
        w->topPC->dl_create_and_add_panel("MPI");
      } else
      {
        fprintf(stderr, "Unknown experiment type.   Try using the IntroWizard.\n");
        exit(0);
      }
    }
#endif // OLDWAY

    w->show();

    qapplication->connect( qapplication, SIGNAL( lastWindowClosed() ), qapplication, SLOT( quit() ) );

    if( splashFLAG )
    {
      splash->raise();
      sleep(1);
      splash->raise();
      splash->message( "Plugins loaded..." );
      sleep(1);
      splash->raise();
      sleep(1);
      splash->raise();
      splash->finish(w);
      delete splash;
      delete splash_pixmap;
    }

    qapplication->exec();

    return;
  }

  // This is simple the routine that is called to start the gui it's own 
  // thread.   Called from Start.cxx, when the gui is requested at startup.
  // Otherwise, it can be called from the command line at any time to 
  // initialize the gui.
  int
  gui_init( void *arg_struct )
  {
      int stat = pthread_create(&phandle[0], 0, (void *(*)(void *))guithreadinit,arg_struct);

      return 1;
  }

}
