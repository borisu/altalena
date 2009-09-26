using iw_voicedb;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Data.SQLite;
using System;

namespace iw_ephonetest
{
    
    
    /// <summary>
    ///This is a test class for UsersTableTest and is intended
    ///to contain all UsersTableTest Unit Tests
    ///</summary>
    [TestClass()]
    public class UsersTableTest
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

        #region Additional test attributes
        // 
        //You can use the following additional attributes as you write your tests:
        //
        //Use ClassInitialize to run code before running the first test in the class
        [ClassInitialize()]
        public static void MyClassInitialize(TestContext testContext)
        {
            _conn = new SQLiteConnection();
            _conn.ConnectionString = @"Data Source=c:\temp\users_test.db;New=True;Synchronous=Off";
            _conn.Open();
        }
        
        //Use ClassCleanup to run code after all tests in a class have run
        [ClassCleanup()]
        public static void MyClassCleanup()
        {
            _conn.Dispose();
            _conn.Close();
        }
        //
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
        #endregion


        /// <summary>
        ///A test for MemoTable Constructor
        ///</summary>
        [TestMethod()]
        [Priority(10)]
        public void UsersTableCreateTest()
        {
            UsersTable target = new UsersTable();
            target.Create(_conn);
        }

        [TestMethod()]
        [Priority(2)]
        public void UsersTableCreateUser()
        {
            UsersTable target = new UsersTable(_conn);
            target.CreateUser(
                "user1", 
                "pw", 
                "a@b", 
                "question", 
                "answer", 
                true, 
                Guid.NewGuid(), 
                "app", 
                DateTime.Now);
            
        }

    }
}
