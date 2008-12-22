using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Office.Interop.Visio;
using System.Windows.Forms;

namespace callfloweditor
{

    [System.Runtime.InteropServices.ComVisible(true)]
    public sealed class EventSink : IVisEventProc
    {

        private Microsoft.Office.Interop.Visio.Application eventApplication;

        private Document eventDocument;

        public EventSink(
            Microsoft.Office.Interop.Visio.Application callingApplication,
            Document callingDocument)
        {
            try
            {
                AddAdvise(callingApplication, callingDocument);
            }
            catch (System.Exception e)
            {
                MessageBox.Show(e.ToString());
            }
            
        }

        [CLSCompliant(false)]
        public void AddAdvise(
             Microsoft.Office.Interop.Visio.Application application,
             Document document)
        {

            const string sink = "";
            const string targetArgs = "";
            Event newEvent = null;

            EventList applicationEvents = application.EventList;
            EventList documentEvents = document.EventList;

            // Shape Added
            newEvent = documentEvents.AddAdvise(
                (unchecked((short)VisEventCodes.visEvtAdd) +
                (short)VisEventCodes.visEvtShape),
                (IVisEventProc)this, sink, targetArgs);

            // Shape Deleted
            newEvent = documentEvents.AddAdvise(
                (short)VisEventCodes.visEvtDel +
                (short)VisEventCodes.visEvtShape,
                (IVisEventProc)this, sink, targetArgs);

            // Connection Added
            newEvent = documentEvents.AddAdvise(
                 (short)VisEventCodes.visEvtConnect +
                 unchecked((short)VisEventCodes.visEvtAdd),
                (IVisEventProc)this, sink, targetArgs);

            // Connection Deleted
            newEvent = documentEvents.AddAdvise(
                 (short)VisEventCodes.visEvtConnect +
                 unchecked((short)VisEventCodes.visEvtDel),
                (IVisEventProc)this, sink, targetArgs);

            newEvent = applicationEvents.AddAdvise(
                (short)VisEventCodes.visEvtApp +
                (short)VisEventCodes.visEvtMarker,
                (IVisEventProc)this, sink, targetArgs);

            newEvent = applicationEvents.AddAdvise(
                (short)VisEventCodes.visEvtApp +
                (short)VisEventCodes.visEvtNonePending,
                (IVisEventProc)this, sink, targetArgs);

            
            return;

        }

        object IVisEventProc.VisEventProc(short eventCode,
            object source,
            int eventId,
            int eventSequenceNumber,
            object subject,
            object moreInfo)
        {

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
                        MessageBox.Show("Fomr:" + cnxn.FromCell.Shape.Name + " To:" + cnxn.ToCell.Shape.Name);

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


