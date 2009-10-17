--
--
--

logger = assert(ivrworx.LOGGER)
conf = assert(ivrworx.CONF)
incoming = assert(ivrworx.INCOMING)

logger:loginfo("welcome");


env = assert(luasql.sqlite3());
conn_string = assert(conf:getstring("polly_db"));
con = assert(env:connect(conn_string));

logger:loginfo("db:"..conn_string.." open");

assert(incoming:answer() == ivrworx.API_SUCCESS);

mrcp = [[<?xml version=\"1.0\"?>
<speak>
  <paragraph>
    <sentence>Welcome to polly application.</sentence>
  </paragraph>
</speak>]]

incoming:speak(mrcp,true);

mrcp = [[<?xml version=\"1.0\"?>
<speak>
  <paragraph>
    <sentence>Please enter user number</sentence>
  </paragraph>
</speak>]]

incoming:speak(mrcp,false);

ivrworx.sleep(20000)


