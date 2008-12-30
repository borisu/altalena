using System;
using System.Collections.Generic;
using System.Text;
using Microsoft.Office.Interop.Visio;



namespace callfloweditor
{

    class CallFlowShape
    {
        Shape _visShape;

        public Shape VisShape
        {
            get { return _visShape; }
            set { _visShape = value; }
        }

        public CallFlowShape(Shape visShape)
        {
            VisShape = visShape;
            validate();
        }

        public const string CallFlowConnector_NameU = "Call Flow Connector";

        public const string Start_NameU = "Start";

        public const string Hangup_NameU = "Hangup";

        public const string Answer_NameU = "Answer";

        public virtual void validate()
        {


            Dictionary<string, bool> connections = new Dictionary<string, bool>();


            foreach (Connect cnxn in VisShape.FromConnects)
            {

                Shape toShape = cnxn.ToCell.Shape;
                if (connections.ContainsKey(toShape.NameU))
                {
                    throw new Exception("shape connected twice to the same shape");
                }

                connections[toShape.NameU] = true;

            }

        }
    }



    class StartShape : CallFlowShape
    {

        public StartShape(Shape visShape)
            : base(visShape)
        {

        }

    }

    class AnswerShape : CallFlowShape
    {

        public AnswerShape(Shape visShape)
            : base(visShape)
        {


        }


    }

    class HangupShape : CallFlowShape
    {
        public HangupShape(Shape visShape)
            : base(visShape)
        {


        }

    }

    class CallFlowconnectorShape : CallFlowShape
    {
        public CallFlowconnectorShape(Shape visShape)
            : base(visShape)
        {
           

        }

        public override void validate()
        {
            base.validate();

            if (VisShape.Connects.Count != 2)
            {
                throw new Exception("connector has to be connected on both ends.");
            }

        }

    }

    static class CallFlowShapeFactory
    {
        public static CallFlowShape CreateInstance(Shape visShape)
        {
            if (visShape.NameU.Equals(CallFlowShape.CallFlowConnector_NameU))
            {
                return new CallFlowconnectorShape(visShape);
            }

            if (visShape.NameU.Equals(CallFlowShape.Answer_NameU))
            {
                return new AnswerShape(visShape);
            }

            if (visShape.NameU.Equals(CallFlowShape.Hangup_NameU))
            {
                return new HangupShape(visShape);
            }

            if (visShape.NameU.Equals(CallFlowShape.Start_NameU))
            {
                return new StartShape(visShape);
            }

            return null;
        }
    }



}
