win = nil;
tid = 0;
gamewnd = nil;

function on_init(args)
	win = toHostWnd(args.sender);
	math.randomseed(os.time());
	--SMessageBox(0,T "execute script function: on_init", T "msgbox", 1);
	return 1;
end

function on_exit(args)
	--SMessageBox(0,T "execute script function: on_exit", T "msgbox", 1);
end

function on_timer(args)
	local r,g,b = math.random(0,255),math.random(0,255),math.random(0,255);
	local strRgb = "rgb(" .. r .."," .. g .. "," .. b ..")";
	canvas:SetAttributeA(SStringA("colorBkgnd",-1),SStringA(strRgb,-1),0);
end


function on_canvas_size(args)
	gamewnd = toSWindow(args.sender);

	local gamecanvas = gamewnd:FindChildByNameA("game_canvas",-1);

	local players = {
					gamecanvas:FindChildByNameA("player_1",-1),
					gamecanvas:FindChildByNameA("player_2",-1),
					gamecanvas:FindChildByNameA("player_3",-1),
					gamecanvas:FindChildByNameA("player_4",-1)
				    };
	local rcCanvas = gamecanvas:GetWindowRect2();
	local heiCanvas = rcCanvas:Height();
	local widCanvas = rcCanvas:Width();

	local rcPlayer =  players[1]:GetWindowRect2();
	local wid = rcPlayer:Width();
	local hei = rcPlayer:Height();

	rcPlayer = CRect(0,0,wid,hei);
	local interval = (heiCanvas - hei*4)/5;
	rcPlayer:OffsetRect(rcCanvas.left,rcCanvas.top+interval);
	for i = 1, 4 do
		players[i]:Move(rcPlayer);
		rcPlayer:OffsetRect(0,interval+hei);
	end

	return 1;

end

function on_run(args)
	if tid == 0 then
		tid = win:setInterval("on_timer",200);
	else
		win:clearTimer(tid);
	end
	return 1;
end

