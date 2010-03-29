using iw_voicedb;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Data.SQLite;
namespace iw_ephonetest
{
    
    
    /// <summary>
    ///This is a test class for MemoTableTest and is intended
    ///to contain all MemoTableTest Unit Tests
    ///</summary>
    [TestClass()]
    public class MemoTableTest
    {


        private TestContext testContextInstance;

        /// <summary>
        ///Gets or sets the test context which provides
        ///information about and functionality for the current test run.
        ///</summary>
        public TestContext TestContext
        {
            get
            {
                return testContextInstance;
            }
            set
            {
                testContextInstance = value;
            }
        }

        private static SQLiteConnection _conn;


        [ClassInitialize()]
        public static void MyClassInitialize(TestContext testContext)
        {

            _conn = new SQLiteConnection();
            _conn.ConnectionString = @"Data Source=c:\temp\memos_test.db;New=True;Synchronous=Off";
            _conn.Open();
        }

        
        //Use ClassCleanup to run code after all tests in a class have run
        [ClassCleanup()]
        public static void MyClassCleanup()
        {
            _conn.Dispose();
            _conn.Close();
        }
        
        //Use TestInitialize to run code before running each test
        //[TestInitialize()]
        //public void MyTestInitialize()
        //{
        //}
        //
        //Use TestCleanup to run code after each test has run
        //[TestCleanup()]
        //public void MyTestCleanup()
        //{
        //}
        //
     


        /// <summary>
        ///A test for MemoTable Constructor
        ///</summary>
        [TestMethod()]
        public void MemoTableCreateTest()
        {
            MemoTable target = new MemoTable();
            target.Create(_conn);
        }

        /// <summary>
        ///A test for MemoTable Constructor
        ///</summary>
        [TestMethod()]
        public void MemoTableInsertMemo()
        {
            MemoTable target = new MemoTable();
            target.InsertMemo(_conn,"user1","key1","value1");
        }
    }
}
