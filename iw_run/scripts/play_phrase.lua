module("play_phrase",package.seeall)


local thousands_sounds = {
		[1]  = "1000.wav",
		[2]  = "2000.wav",
		[3]  = "3000.wav",
		[4]  = "4000.wav",
		[5]  = "5000.wav",
		[6]  = "6000.wav",
		[7]  = "7000.wav",
		[8]  = "8000.wav",
		[9]  = "9000.wav",
		[10] = "10000.wav",
}

local hundreds_sounds = {
		[100]  = "100.wav",
		[200]  = "200.wav",
		[300]  = "300.wav",
		[400]  = "400.wav",
		[500]  = "500.wav",
		[600]  = "600.wav",
		[700]  = "700.wav",
		[800]  = "800.wav",
		[900]  = "900.wav",
		}


local two_digits_sounds = {
		[1]  = "1.wav",
		[2]  = "2.wav",
		[3]  = "3.wav",
		[4]  = "4.wav",
		[5]  = "5.wav",
		[6]  = "6.wav",
		[7]  = "7.wav",
		[8]  = "8.wav",
		[9]  = "9.wav",
		[10] = "10.wav",
		----------------------------------------------------------------------- 10
		[11] = "11.wav",
		[12] = "12.wav",
		[13] = "13.wav",
		[14] = "14.wav",
		[15] = "15.wav",
		[16] = "16.wav",
		[17] = "17.wav",
		[18] = "18.wav",
		[19] = "19.wav",
		[20] = "20.wav",
		----------------------------------------------------------------------- 20
		[21] = "21.wav",
		[22] = "22.wav",
		[23] = "23.wav",
		[24] = "24.wav",
		[25] = "25.wav",
		[26] = "26.wav",
		[27] = "27.wav",
		[28] = "28.wav",
		[29] = "29.wav",
		[30] = "30.wav",
		----------------------------------------------------------------------- 30
		[31] = "31.wav",
		[32] = "32.wav",
		[33] = "33.wav",
		[34] = "34.wav",
		[35] = "35.wav",
		[36] = "36.wav",
		[37] = "37.wav",
		[38] = "38.wav",
		[39] = "39.wav",
		[40] = "40.wav",
		----------------------------------------------------------------------- 40
		[41] = "41.wav",
		[42] = "42.wav",
		[43] = "43.wav",
		[44] = "44.wav",
		[45] = "45.wav",
		[46] = "46.wav",
		[47] = "47.wav",
		[48] = "48.wav",
		[49] = "49.wav",
		[50] = "50.wav",
		----------------------------------------------------------------------- 50
		[51] = "51.wav",
		[52] = "52.wav",
		[53] = "53.wav",
		[54] = "54.wav",
		[55] = "55.wav",
		[56] = "56.wav",
		[57] = "57.wav",
		[58] = "58.wav",
		[59] = "59.wav",
		[60] = "60.wav",
		----------------------------------------------------------------------- 60
		[61] = "61.wav",
		[62] = "62.wav",
		[63] = "63.wav",
		[64] = "64.wav",
		[65] = "65.wav",
		[66] = "66.wav",
		[67] = "67.wav",
		[68] = "68.wav",
		[69] = "69.wav",
		[70] = "70.wav",
		----------------------------------------------------------------------- 70
		[71] = "71.wav",
		[72] = "72.wav",
		[73] = "73.wav",
		[74] = "74.wav",
		[75] = "75.wav",
		[76] = "76.wav",
		[77] = "77.wav",
		[78] = "78.wav",
		[79] = "79.wav",
		[80] = "80.wav",
		----------------------------------------------------------------------- 80
		[81] = "81.wav",
		[82] = "82.wav",
		[83] = "83.wav",
		[84] = "84.wav",
		[85] = "85.wav",
		[86] = "86.wav",
		[87] = "87.wav",
		[88] = "88.wav",
		[89] = "89.wav",
		[90] = "90.wav",
		----------------------------------------------------------------------- 90
		[91] = "91.wav",
		[92] = "92.wav",
		[93] = "93.wav",
		[94] = "94.wav",
		[95] = "95.wav",
		[96] = "96.wav",
		[97] = "97.wav",
		[98] = "98.wav",
		[99] = "99.wav",
		----------------------------------------------------------------------- 99
}

local function play_zero(handle)

	return ivrworx.play(handle,numbers_path.."\\0.wav",true,false);
	
end

local function play_minus(handle)

	return ivrworx.play(handle,numbers_path.."\\minus.wav",true,false);
	
end

local function play_and(handle)

	-- not implemented currently

end


local function play_hundreds(handle,i)

	if (i == 0 or i > 10) then return 0; end;
	
	return ivrworx.play(handle,numbers_path.."\\"..hundreds_sounds[i*100],true,false);
	
end


local function play_two_digits(handle,i)

	if (i == 0) then return 0; end;
		
	return ivrworx.play(handle,numbers_path.."\\"..two_digits_sounds[i],true,false);

end

local function play_three_digits(handle, i, radix)

	if ( i==0 ) then return 0; end;
		
		local last_two_digits =  i % 100;
		local hundreds = i - last_two_digits;

		if (hundreds ~= 0)
  		then
			res = play_hundreds(handle,hundreds);
			if (res ~= 0) then return res; end
			 
			res = play_and(handle);
			if (res ~= 0) then return res; end
  		end

		res = play_two_digits(handle,last_two_digits);
		if (res ~= 0) then return res; end
		
		if (radix ~= nil and radix ~= "" ) then 
		
			res = ivrworx.play(handle,numbers_path.."\\"..radix,true,false);
			if (res ~= 0) then return res; end
			
		end
end


function play_number(handle,i) 

	local num_to_play  = math.abs(i);
	
	if (conf["sounds_dir"] == nil) then 
		numbers_path = "basic_words\\numbers";
	else
		numbers_path = conf["sounds_dir"].."\\basic_words\\numbers";
	end
	

	if  (i == 0) 
	then
	  return  play_zero(handle)
	end

	if (i < 0)
	then
	  return play_minus(handle);
	end


	--
	-- int range  0 to 4,294,967,295
	--
	local num_str = tostring(num_to_play);
	local num_len = string.len(num_str);
	if (num_len > 10) then return end;
	
	
	
	if (num_len < 10) then
	 zeroes = string.rep("0", 10 - num_len);
	 num_str = zeroes..num_str;
	end 

	--
	-- play milliards
	--
	res = play_three_digits(handle,tonumber(string.sub(num_str,1,1)),"Billion.wav");
	if (res ~= 0) then return res; end;
	
	
	--
	-- play millions
	--
	res = play_three_digits(handle,tonumber(string.sub(num_str,2,4)),"Million.wav");
	if (res ~= 0) then return res; end;
	
			
	--
	-- play thousands		
	-- 
	-- some thousands are prerecorded
	
	thousands_num = tonumber(string.sub(num_str,5,7));
	if (thousands_num >=1 and thousands_num <= 10) then 
	
		res = ivrworx.play(handle,numbers_path.."\\"..thousands_sounds[thousands_num],true,false);
		if (res ~= 0) then return res; end;
		
	else
	
	 	res = play_three_digits(handle,thousands_num,"Thousand.wav");
	 	if (res ~= 0) then return res; end;
	 	
	end
	
	--
	-- play remainder		
	-- 
	return play_three_digits(handle,tonumber(string.sub(num_str,8,10)));
		


end


--
-- Checks only first symbol for alphanumericity
--
function isalphanumeric(s)

	if (s == nil or string.len(s)== 0) then return false; end;

	local asciicode = tonumber(string.byte(s))

	if  ( (asciicode >= 48 and asciicode <= 57) or
		  (asciicode >= 65 and asciicode <= 90) or
		  (asciicode >= 97 and  asciicode <= 122))
	then
		return true
	else
		return false
	end

end


function spell(handle, str)

	if (str == nil) then return -1; end;
	
	if (conf["sounds_dir"] == nil) then 
		letters_path = "basic_words\\alphabet";
	else
		letters_path = conf["sounds_dir"].."\\basic_words\\alphabet";
	end
	
	local num_len = string.len(str);
	
	for i = 1,num_len,1 do
	
	    local symbol = string.sub(str,i,i);
	    
	    if (tonumber(symbol) ~= nil) then
	    	play_number(handle,tonumber(symbol))
	    else
			if (isalphanumeric(symbol)) then
				ivrworx.play(handle,letters_path.."\\"..symbol..".wav",true,false);
			end
		end
	    
	end

end


-- test begin


--function play(file,sync,loop)
--	print (file)
--end

--ivrworx = { ["play"] = play }
--conf = { ["sounds_dir"] = "." }



--spell (1234)

--print("--123");
--spell(nil);
 

 
--
--
--print("one digit ----")
--print(0);play_number(0)
--print(1);play_number(1)
--print(5);play_number(5)
--
--print("two digit ----")
--print(10);play_number(10)
--print(11);play_number(11)
--print(15);play_number(15)
--
--print("three digit ----")
--print(100);play_number(100)
--print(101);play_number(101)
--print(199);play_number(199)
--
--print("four digit ----")
--print(1000);play_number(1000)
--print(1001);play_number(1001)
--print(1910);play_number(1910)
--
--print("five digit ----")
--print(10000);play_number(10000)
--
--print("six digit ----")
--print(100000);play_number(100000)
--
--print("seven digit ----")
--print(1000011);play_number(1000011)
--
--print("eight digit ----")
--print(10000111);play_number(10000111)
--
--print("nine digit ----")
--print(100001111);play_number(100001111)
--
--print("ten digit ----")
--print(9100001111);play_number(9100001111)
--
--print("negative digit ----")
--print(-9100001111);play_number(-9100001111)




-- test end