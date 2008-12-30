using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Office.Interop.Visio;
using System.Windows.Forms;

namespace callfloweditor
{

    [System.Runtime.InteropServices.ComVisible(true)]
    public sealed class AltalenaEventSink : IVisEventProc
    {
        /// <summary>Visio.Application object.</summary>
        private Microsoft.Office.Interop.Visio.Application
                eventApplication;

        /// <summary>Visio.Document object.</summary>
        private Document eventDocument;
        
        /// <summary>AddAdvise method takes the Visio application
        /// and document and adds events for this solution to
        /// their event lists.</summary>
        /// <param name="callingApplication">The Visio application that
        /// the user will be working with.</param>
        /// <param name="callingDocument">The Visio document that the user
        /// will be working with.</param>
        [CLSCompliant(false)]
        public void AddAdvise(
            Microsoft.Office.Interop.Visio.Application callingApplication,
            Document callingDocument)
        {

            // The calling application must exist.
            if (callingApplication == null)
            {

                // Throw a meaningful error.
                throw new ArgumentNullException();
            }

            // The calling document must exist.
            if (callingDocument == null)
            {

                // Throw a meaningful error.
                throw new ArgumentNullException();
            }

            // Save the application for setting the events.
            eventApplication = callingApplication;


            // Save the document for setting the events.
            eventDocument = callingDocument;

            // Add events of interest.
            setAddAdvise();

            return;
        }

       /// <summary>The SetAddAdvise method adds the events that need to
		/// be responded to with solution-specific behavior to the Visio
		/// objects.  For this sample, the add and delete shape events are
		/// handled so that the data in the grid can be kept consistent
		/// with the drawing.  Marker events are handled to respond to shape
		/// double-click from shapes in the sample office furniture stencil.
		/// The none-pending event is also handled so that the add and delete
		///  processing will only be done when Visio is idle.</summary>
		/// <remarks>An Exception is thrown on failure.</remarks>
		private void setAddAdvise()
        {

            // The Sink and TargetArgs values aren't needed.
// 			const string sink = "";
// 			const string targetArgs = "";
// 
// 			Event newEvent = null;


            EventList applicationEvents = eventApplication.EventList;
            EventList documentEvents = eventDocument.EventList;

            return;

// 			// Add the shape-added event to the document. The new shape
// 			// will be available for processing in the handler.  The
// 			// value for VisEventCodes.visEvtAdd cannot be
// 			// automatically converted to a short type, so the
// 			// unchecked function is used.  This allows the addition to
// 			// be done and returns a valid short value.
//             newEvent = eventDocument.EventList.AddAdvise(
// 				(unchecked((short)VisEventCodes.visEvtAdd) +
// 				(short)VisEventCodes.visEvtShape),
// 				(IVisEventProc)this, sink, targetArgs);
// 
// 			// Add the before-shape-deleted event to the document.  This 
// 			// event will be raised when a shape is deleted from the
// 			// document. The deleted shape will still be available for
// 			// processing in the handler.
// 			newEvent = documentEvents.AddAdvise(
// 				(short)VisEventCodes.visEvtDel +
// 				(short)VisEventCodes.visEvtShape,
// 				(IVisEventProc)this, sink, targetArgs);
// 
// 			// Add marker events to the application.  This event
// 			// will be raised when a user double-clicks a shape from
// 			// the sample office furniture stencil.
// 			newEvent = applicationEvents.AddAdvise(
// 				(short)VisEventCodes.visEvtApp +
// 				(short)VisEventCodes.visEvtMarker,
// 				(IVisEventProc)this, sink, targetArgs);
// 
// 			// Add the none-pending event to the application.  This
// 			// event will be raised when Visio is idle.
// 			newEvent = applicationEvents.AddAdvise(
// 				(short)VisEventCodes.visEvtApp +
// 				(short)VisEventCodes.visEvtNonePending, 
// 				(IVisEventProc)this, sink, targetArgs);
// 
//             
// 			return;

        }

        object IVisEventProc.VisEventProc(short eventCode,
            object source,
            int eventId,
            int eventSequenceNumber,
            object subject,
            object moreInfo)
        {
            return null;

            Microsoft.Office.Interop.Visio.Application eventProcApplication = null;
            Document eventProcDocument = null;
            Shape eventShape = null;

            // Get the application or document passed in.
            if (source is IVApplication)
            {

                eventProcApplication =
                    (Microsoft.Office.Interop.Visio.Application)source;
            }
            else if (source is IVDocument)
            {

                eventProcDocument = (Document)source;
            }

            // Check for each event code that is handled.  The event
            // codes are a combination of an object and an action.
            // Only the events added in the SetAddAdvise method will
            // be sent to this method, and only those events need to
            // be included in this switch statement.
            switch (eventCode)
            {

                // The value for VisEventCodes.visEvtAdd cannot be
                // automatically converted to a short type.  The
                // unchecked function allows the addition to be
                // done and returns a valid short value.
                case (short)VisEventCodes.visEvtShape +
                unchecked((short)VisEventCodes.visEvtAdd):

                    eventShape = (Shape)subject;
                    break;

                case (short)VisEventCodes.visEvtDel +
                (short)VisEventCodes.visEvtShape:

                    eventShape = (Shape)subject;
                    break;

                case (short)VisEventCodes.visEvtApp +
                (short)VisEventCodes.visEvtMarker:

                    break;

                case (short)VisEventCodes.visEvtApp +
                (short)VisEventCodes.visEvtNonePending:

                    break;

                case (short)VisEventCodes.visEvtConnect +
                 unchecked((short)VisEventCodes.visEvtAdd):
                    // Subject object is a Connects collection
                    Connects subjectConnects = (Connects)subject;
                    int s = subjectConnects.Count;

                   
                    foreach(Connect cnxn in  subjectConnects)
                    {
                        MessageBox.Show("From:" + cnxn.FromCell.Shape.Name + " To:" + cnxn.ToCell.Shape.Name);

                    }
                    

                    
                    break;

                case (short)VisEventCodes.visEvtConnect +
                 unchecked((short)VisEventCodes.visEvtDel):
                    // Subject object is a Connects collection
//                     Connects subjectConnects = (Connects)subject;
//                     int s = subjectConnects.Count;


                    break;

                default:
                    break;
            }

            return null;
        }
    }
}


