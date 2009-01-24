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

        public const string CallFlowConnector_MasterName = "Call Flow Connector";

        public const string Start_MasterName             = "Start";

        public const string Hangup_MasterName            = "Hangup";

        public const string Answer_MasterName            = "Answer";

        public virtual string GenerateFunctionBlock()
        {
            throw new Exception("This operation is not implemented for this shape.");
        }

        public virtual void validate()
        {
            throw new Exception("This operation is not implemented for this shape.");
        }

        public virtual string implname()
        {
            throw new Exception("This operation is not implemented for this shape.");

        }

        public virtual string GetFixedName()
        {
            string dottedName = VisShape.NameU;
            return dottedName.Replace(".", "_");
        }

    }

    class ShapeWithSingleConnection : 
        CallFlowShape
    {
        public ShapeWithSingleConnection(Shape shape):
            base(shape){}

        public override void validate()
        {
            if ( VisShape.Connects.Count > 2)
            {
              throw new Exception("call flow shape is not properly connected");
            }
        }
        
        public virtual Shape GetNextShape()
        {
            
            if (VisShape.FromConnects.Count == 0)
            {
                return null;
            }

            // Find the Connect for which this shape is start
            // For connector object itself FromConnects property 
            // should be null
            Connect startConnection = null;
            foreach (Connect cnxn  in VisShape.FromConnects)
            {
                if (cnxn.FromPart  == (int) VisFromParts.visBegin)
                {
                    if (startConnection == null)
                    {
                        startConnection = cnxn;
                    } 
                    else
                    {
                        throw new Exception("This connection can only be source for only one other block");
                    }
                }
            }

            if (startConnection == null)
            {
                return null;
            }

            // look for connections inside connector object
            // and fine the one which is end
            Shape connectorShape = startConnection.FromSheet;
            Shape destShape = null;
            foreach (Connect cnxn in connectorShape.Connects)
            {
                if (cnxn.FromPart == (int)VisFromParts.visEnd)
                {
                    if (destShape == null)
                    {
                        destShape = cnxn.ToSheet;
                        break;
                    }
                }
            }

            return destShape;

        }


        public override string GenerateFunctionBlock()
        {
           string myFixedName = GetFixedName();
           string gluedShapeFixedName = "nil";

           Shape nextVisShape = GetNextShape();
           if (nextVisShape != null)
           {
               CallFlowShape nextCallFlowShape = CallFlowShapeFactory.CreateInstance(nextVisShape);
               if (nextCallFlowShape != null)
               {
                   gluedShapeFixedName = nextCallFlowShape.GetFixedName();
               }
           }
           return "function " + myFixedName + "() " + implname() +  "(this); return " + gluedShapeFixedName + "; end;";
        }

    }


    class StartShape : 
        ShapeWithSingleConnection
    {

        public StartShape(Shape visShape)
            : base(visShape)
        {

        }

        public override string GenerateFunctionBlock()
        {
            return "function start() return " + (GetNextShape() == null ? "nil" : GetNextShape().NameU)  + "; end;";
        }

    }

    class AnswerShape : 
        ShapeWithSingleConnection
    {

        public AnswerShape(Shape visShape)
            : base(visShape)
        {


        }

        public override string implname()
        {
            return "this.answer";
        }

    }

    class HangupShape :
        ShapeWithSingleConnection
    {
        public HangupShape(Shape visShape)
            : base(visShape)
        {


        }

        public override string implname()
        {
            return "this.hangup";
        }

    }

    class CallFlowconnectorShape :
        ShapeWithSingleConnection
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

        public override string GenerateFunctionBlock()
        {
            return null;
        }

    }

    static class CallFlowShapeFactory
    {
        public static CallFlowShape CreateInstance(Shape visShape)
        {
            string masterName = visShape.Master.Name;

            if (masterName.Equals(CallFlowShape.CallFlowConnector_MasterName))
            {
                return new CallFlowconnectorShape(visShape);
            }

            if (masterName.Equals(CallFlowShape.Answer_MasterName))
            {
                return new AnswerShape(visShape);
            }

            if (masterName.Equals(CallFlowShape.Hangup_MasterName))
            {
                return new HangupShape(visShape);
            }

            if (masterName.Equals(CallFlowShape.Start_MasterName))
            {
                return new StartShape(visShape);
            }

            return null;
        }
    }



}
