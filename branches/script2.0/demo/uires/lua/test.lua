win = nil;
tid = 0;
gamewnd = nil;

function on_init(args)
	win = toHostWnd(args.sender);
	math.randomseed(os.time());
	--SMessageBox(0,T "execute script function: on_init", T "msgbox", 1);
end

function on_exit(args)
	--SMessageBox(0,T "execute script function: on_exit", T "msgbox", 1);
end

function on_timer(args)
	if(gamewnd ~= nil) then

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

	for i = 1,4 do
		local prog = players[i]:GetUserData();
		if(prog<100) then
			prog = prog + math.random(0,5);
			players[i]:SetUserData(prog);
			local rc = players[i]:GetWindowRect2();
			rc.left = rcCanvas.left + (widCanvas-wid)*prog/100;
			players[i]:Move2(rc.left,rc.top,-1,-1);
		end
	end

	end

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
		local rc = rcPlayer;
		if(tid ~= 0) then
			rc.left = (widCanvas-wid)*players[i]:GetUserData()/100;
		end
		players[i]:Move2(rc.left,rc.top,-1,-1);
		rcPlayer:OffsetRect(0,interval+hei);
	end

	return 1;

end

function on_run(args)
	local btn = toSWindow(args.sender);
	if tid == 0 then
		tid = win:setInterval("on_timer",200);
		btn:SetWindowText(T"stop");

	else
		win:clearTimer(tid);
		btn:SetWindowText(T"run");
		tid = 0;
	end
	return 1;
end
