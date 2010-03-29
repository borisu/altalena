using System;
using System.Collections.Generic;
using System.Text;
using Visio = Microsoft.Office.Interop.Visio;
 



namespace callfloweditor
{

    class CallFlowShape
    {
        Visio.Shape _visShape;

        public Visio.Shape VisShape
        {
            get { return _visShape; }
            set { _visShape = value; }
        }


        public CallFlowShape(Visio.Shape visShape)
        {
            VisShape = visShape;
            validate();
        }

        public const string CallFlowConnector_MasterName = "Call Flow Connector";
        public const string Start_MasterName             = "Start";
        public const string Hangup_MasterName            = "Hangup";
        public const string Wait_MasterName              = "Wait.8";
        public const string Answer_MasterName            = "Answer";

        public virtual string GenerateFunctionBlock()
        {
            throw new Exception("This operation is not implemented for this shape.");
        }

        public virtual string GenerateArgumentsList()
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

    static class CallFlowShapeFactory
    {
        public static CallFlowShape CreateInstance(Visio.Shape visShape)
        {

            if (visShape == null || visShape.Master == null)
            {
                return null;
            }

            string masterName = visShape.Master.Name;

            if (masterName.Equals(CallFlowShape.Start_MasterName))
            {
                return new StartShape(visShape);
            }

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

            if (masterName.Equals(CallFlowShape.Wait_MasterName))
            {
                return new WaitShape(visShape);
            }

            return null;
        }
    }

    class ShapeWithSingleConnection : 
        CallFlowShape
    {
        public ShapeWithSingleConnection(Visio.Shape shape)
            :
            base(shape){}

        public override void validate()
        {
            if ( VisShape.Connects.Count > 2)
            {
              throw new Exception("call flow shape is not properly connected");
            }
        }

        public virtual Visio.Shape GetNextShape()
        {
            
            if (VisShape.FromConnects.Count == 0)
            {
                return null;
            }

            // Find the Connect for which this shape is start
            // For connector object itself FromConnects property 
            // should be null
            Visio.Connect startConnection = null;
            foreach (Visio.Connect cnxn in VisShape.FromConnects)
            {
                if (cnxn.FromPart == (int)Visio.VisFromParts.visBegin)
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
            Visio.Shape connectorShape = startConnection.FromSheet;
            Visio.Shape destShape = null;
            foreach (Visio.Connect cnxn in connectorShape.Connects)
            {
                if (cnxn.FromPart == (int)Visio.VisFromParts.visEnd)
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

        public override string GenerateArgumentsList()
        {
            return "(this)";
        }


        public override string GenerateFunctionBlock()
        {
           string myFixedName = GetFixedName();
           string gluedShapeFixedName = "nil";

           Visio.Shape nextVisShape = GetNextShape();
           if (nextVisShape != null)
           {
               CallFlowShape nextCallFlowShape = CallFlowShapeFactory.CreateInstance(nextVisShape);
               if (nextCallFlowShape != null)
               {
                   gluedShapeFixedName = nextCallFlowShape.GetFixedName();
               }
           }
           return "function " + myFixedName + "() " + implname() + GenerateArgumentsList() + "; return " + gluedShapeFixedName + "; end;";
        }

    }


    class StartShape : 
        ShapeWithSingleConnection
    {

        public StartShape(Visio.Shape visShape)
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

        public AnswerShape(Visio.Shape visShape)
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
        public HangupShape(Visio.Shape visShape)
            : base(visShape)
        {


        }

        public override string implname()
        {
            return "this.hangup";
        }

    }

    class WaitShape :
       ShapeWithSingleConnection
    {
        private int _timeToWait;

        public int TimeToWait
        {
            get { return _timeToWait; }
            set { _timeToWait = value; }
        }

        public override string GenerateArgumentsList()
        {
            return "(this," + TimeToWait +")";
        }


        public WaitShape(Visio.Shape visShape)
            : base(visShape)
        {

            Visio.Cell cell = visShape.get_Cells("Prop.Row_1");
            TimeToWait = cell.get_ResultInt(
                Visio.VisUnitCodes.visNumber,
                (short)Visio.VisRoundFlags.visRound); ;
        }

        public override string implname()
        {
            return "this.wait";
        }

    }

    class CallFlowconnectorShape :
        ShapeWithSingleConnection
    {
        public CallFlowconnectorShape(Visio.Shape visShape)
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

    



}
