-- FUNCTIONS DEFS START --
                          
function start() return Answer; end;
                          
                          
function Answer() this.answer(this); return nil; end;
                          
-- FUNCTIONS DEFS END --  
                          
                          
-- MAIN BLOCK START --    
nextfunc = start();       
while (nextfunc ~= nil) do
   nextfunc = nextfunc(); 
end;                      
-- MAIN BLOCK END --      

