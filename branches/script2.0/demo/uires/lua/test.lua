win = nil;
tid = 0;
canvas = nil;

function on_init(args)
	win = toHostWnd(args.sender);
	math.randomseed(os.time());
	SMessageBox(0,T "execute script function: on_init", T "msgbox", 1);
	return 1;
end

function on_exit(args)
	SMessageBox(0,T "execute script function: on_exit", T "msgbox", 1);
end

function on_timer(args)
	local r,g,b = math.random(0,255),math.random(0,255),math.random(0,255);
	local strRgb = "rgb(" .. r .."," .. g .. "," .. b ..")";
	canvas:SetAttributeA(SStringA("colorBkgnd",-1),SStringA(strRgb,-1),0);	
end

function onEvtTest2(args)
	if tid == 0 then
		tid = win:setInterval("on_timer",200);
	else
		win:clearTimer(tid);
	end
	return 1;
end

function onEvtTstClick(args)
	local txt3=SStringW(L"append",-1);
	local sender=toSWindow(args.sender);
	canvas = sender:GetParent();
	canvas:CreateChildrenFromString(L"<button pos=\"0,0,150,30\" name=\"btn_script\" on_command=\"onEvtTest2\">lua btn 中文</button>");
	sender:SetVisible(0,1);
	return 1;
end
