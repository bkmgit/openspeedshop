/*! \class Frame
    The Frame class is the container class for all user PanelContainers.   
    The events handled are:
    - contextMenuEvent
      QContextMenuEvent is passed both to the PanelContainer to create
      it's menu, but also to the Panel to add any additional Panel specifi
      menu options.
    - resizeEvent
      This is passed trough to the Frame's parent PanelContainer for fielding.
    - dropEnterEvent
      Currently called but nothing happens
    - dropEnter
      This is the worker event for the drop event.
 */


#include "Frame.hxx"

#include "PanelContainer.hxx"
#include "DragNDropPanel.hxx"

#include <qdragobject.h>

#include <qpopupmenu.h>

#include <qcursor.h>

#define DEBUG_OUTPUT_REQUESTED 1
#include "debug.hxx"  // This includes the definition of nprintf(DEBUG_FRAMES) 

#include <stdlib.h>  // for the free() call below.

bool Frame::dragging = FALSE;

/*! This constructor should not be called. */
Frame::Frame( )
{
  fprintf(stderr, "This constructor should not be called.\n");
  fprintf(stderr, "see: Frame::Frame(PanelContainer *, QWidget *, const char *);\n");
  dragEnabled = FALSE;
  dropEnabled = FALSE;
  panelContainer = NULL;
}

/*! Work constructor.   Set's the name of the frame, the pointer to the
    parent panel container, and the frame shape and shadow characteristics.

    \param pc is a pointer to PanelContainer it is attached to.
    \param parent is the parent widget to attach this Frame.
    \param name is the name give to the Frame.
 */
Frame::Frame( PanelContainer *pc, QWidget *parent, const char *n )
    : QFrame( parent, n )
{
  char tmp_name[1024];

  setAcceptDrops(TRUE);

  sprintf(tmp_name, "%s: %s", pc->getInternalName(), n);
  name = strdup(tmp_name);
  dragEnabled = FALSE;
  dropEnabled = FALSE;
  setFrameShape( QFrame::StyledPanel );
  setFrameShadow( QFrame::Raised );
  panelContainer = pc;

  languageChange();
}

/*! Destroys the object and frees any allocated resources, namely name.
 */
Frame::~Frame( )
{
  if( name )
  {
    free(name);
  }
}

#ifdef OLD_DRAG_AND_DROP
/*! Fields the mousePressEvent.
    This is how drag is enabled for a panel.  (if the dragEnable flag is set.)
    The sourceDragNDropObject is created and a global flag is set notifying 
    everyone that a drag is undeway.
 */
void Frame::mousePressEvent(QMouseEvent *e)
{
  if( panelContainer == NULL )
  {
    return;
  }

  nprintf(DEBUG_FRAMES) ("Frame::mousePressEvent(%s) pc=(%s)\n", getName(), panelContainer->getInternalName() );

  if( e->button() != LeftButton )
  {
    return;
  }

  if( !dragEnabled )
  {
      nprintf(DEBUG_FRAMES) (" dragging not enable for this frame\n");
    return;
  }
  nprintf(DEBUG_FRAMES) ("attempt to drag the panel \n");

  Frame::dragging = TRUE;

  DragNDropPanel::sourceDragNDropObject = new DragNDropPanel(panelContainer, this);

  if( DragNDropPanel::sourceDragNDropObject == NULL )
  {
    Frame::dragging = FALSE;
  }
}

/*! Fields the mouseReleaseEvent.
    This is how the Panel drop is activated.   
    If a drag is occuring, then the DragNDropPanel::DropPanel() routine 
    is called to handle dropping of the Panel.
 */
void Frame::mouseReleaseEvent(QMouseEvent *)
{
  if( panelContainer == NULL )
  {
    return;
  }

  nprintf(DEBUG_FRAMES) ("Frame::mouseReleaseEvent(%s-%s) dropEnabled=%d Frame::dragging=%d entered\n", panelContainer->getInternalName(), panelContainer->getExternalName(), dropEnabled, Frame::dragging );

  if( !dropEnabled || !Frame::dragging )
  {
    return;
  }
  nprintf(DEBUG_FRAMES) ("attempt to drop the panel \n");

  // Change the cursor... back
  nprintf(DEBUG_FRAMES) (" change the cursor back\n");
  Frame::dragging = FALSE;

  DragNDropPanel::sourceDragNDropObject->DropPanel(panelContainer);
}
#endif // OLD_DRAG_AND_DROP

#ifndef OLD_DRAG_AND_DROP
/*! This callback is currently unused.   */
void Frame::dragEnterEvent(QDragEnterEvent* event)
{
  nprintf(DEBUG_FRAMES) ("Frame::dragEnterEvent() entered\n");

  QString text;
  if( QTextDrag::canDecode(event) )
  {
    if( QTextDrag::decode(event, text) )
    {
      nprintf(DEBUG_FRAMES) ("text=(%s)\n", text.ascii() );
      if( strcmp(text.ascii(), "OpenSpeedShop-QDragString") == 0 )
      {
        event->accept( QTextDrag::canDecode(event) );
      }
    }
  }
}

/*! This callback is generated from the Qt Drag and Drop event.   Once fielded
    it will call a routine to determine where the drop event occurred and
    then act on it.
 */
void Frame::dropEvent(QDropEvent* event)
{
  nprintf(DEBUG_FRAMES) ("Frame::dropEnterEvent() entered\n");
  nprintf(DEBUG_FRAMES) ("Frame::dropEvent(%s-%s) dropEnabled=%d Frame::dragging=%d entered\n", panelContainer->getInternalName(), panelContainer->getExternalName(), dropEnabled, Frame::dragging );

  if( !dropEnabled || !Frame::dragging )
  {
    return;
  }
  nprintf(DEBUG_FRAMES) ("attempt to drop the panel \n");

  // Change the cursor... back
  nprintf(DEBUG_FRAMES) (" change the cursor back\n");
  Frame::dragging = FALSE;

  DragNDropPanel::sourceDragNDropObject->DropPanelWithQtDnD(panelContainer);
}
#endif // OLD_DRAG_AND_DROP

/*! This event is passed on to the PanelContainer::resizeEvent(...) routine.
 */
void Frame::resizeEvent(QResizeEvent *e)
{ 
  if( !panelContainer )
  {
    return;
  }

  panelContainer->handleSizeEvent(e);
//  nprintf(DEBUG_FRAMES) ("Frame::resizeEvent()s\n");
} 


/*! Fields the mouseReleaseEvent.
    This is how the Panel drop is activated.   
    If a drag is occuring, then the DragNDropPanel::DropPanel() routine 
    is called to handle dropping of the Panel.
 */
/*! Fields the menu event to create the PanelContainer menu, then 
    request the current (raised) Panel add it's menu entries. */
void
Frame::contextMenuEvent( QContextMenuEvent * )
{
  bool *flag = NULL;
  if( panelContainer == NULL )
  {
    nprintf(DEBUG_FRAMES) ("  no panelContainer (ERROR).  - return\n");
    return;
  }
  if( panelContainer->leftPanelContainer && panelContainer->rightPanelContainer)
  {  // There should be no menu action for this split panel container.  Only
     // for it's children.
    nprintf(DEBUG_FRAMES) ("  There are children.  - return\n");
     return;
  }
  if( panelContainer->_masterPC->_doingMenuFLAG == TRUE )
  {
    nprintf(DEBUG_FRAMES) ("  already doing menu - return\n");
    return;
  }
  panelContainer->_masterPC->_doingMenuFLAG = TRUE;
  flag = &panelContainer->_masterPC->_doingMenuFLAG;


  nprintf(DEBUG_FRAMES) ("Frame::contextMenuEvent(%s-%s)\n",
    panelContainer->getInternalName(), panelContainer->getExternalName() );

  panelContainer->_masterPC->panelContainerContextMenuEvent( panelContainer, FALSE );

  if( flag ) *flag = FALSE;
}


#ifdef OLDWAY
void Frame::enterEvent ( QEvent * )
{
  nprintf(DEBUG_FRAMES) ("Frame::enterEvent(%s) pc=(%s-%s)\n", getName(), panelContainer->getInternalName(), panelContainer->getExternalName() );
}


// Not active at this time, but it's the hook to handle leave events.
void Frame::leaveEvent ( QEvent * )
{
//  nprintf(DEBUG_FRAMES) ("Frame::leaveEvent(%s) pc=(%s)\n", getName(), panelContainer->getInternalName() );
}
#endif // OLDWAY

/*!  Sets the strings of the subwidgets using the current language.
 */
void
Frame::languageChange()
{ 
  setCaption( tr( name ) );
} 



