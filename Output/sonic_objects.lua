-- by r57shell

gui.register(function ()
	local camerax = memory.readword(0xFFEE78);
	local cameray = memory.readword(0xFFEE7C);
	local j = 0;
	-- [[
	for i = 0xFFB000, 0xFFCFCB-1, 0x4A do
		local code = memory.readlong(i)
		if (code ~= 0) then
			local x,y,w,h = memory.readword(i+0x10),memory.readword(i+0x14),memory.readbyte(i+7),memory.readbyte(i+6);
			x = x - camerax;
			y = y - cameray;
			gui.box(x-w/2,y-h/2,x+w/2,y+h/2);
			gui.text(x-w/2,y-h/2,string.format("%06X %06X",i,code));
			j = j + 1;
		end
	end
	--]]
	
	j = 0;
	-- [[
	local current = memory.readlong(0xFFF772);
	local i = current;
	local start = 0;
	while ( i >= 0 and i > current - 800*6 ) do
		if (memory.readlong(i+2) == 0) then
			start = i;
			break;
		end
		i = i - 6;
	end

	i = current;
	while ( i <= 0x400000 and i < current + 800*6 ) do
		if (memory.readword(i) == 0xFFFF) then
			_end = i;
			break;
		end
		i = i + 6;
	end
	
	gui.text(0,0,string.format("%02X %02X",start,_end));
	if (start ~= 0 and _end ~= 0) then
		for i = start, _end-1, 6 do
			--if (memory.readbyte(i+4) ~= 2) then
				--gui.text(0,j*8,string.format("%02X %02X",memory.readbyte(i+4),memory.readbyte(i+5)));
				local x = memory.readword(i);
				local y = AND(memory.readword(i+2),0xFFF);
				x = x - camerax;
				y = y - cameray;
				--gui.box(x-w/2,y-h/2,x+w/2,y+h/2);
				gui.text(x,y,string.format("%02X %02X",memory.readbyte(i+4),memory.readbyte(i+5)));
				j = j + 1;
			--end
		end
	end
	--]]
end)
