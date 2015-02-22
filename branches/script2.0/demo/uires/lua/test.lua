function on_init(args)
	SMessageBox(0,T "execute script function: on_init", T "msgbox", 1);
end

function on_exit(args)
	SMessageBox(0,T "execute script function: on_exit", T "msgbox", 1);
end

function onEvtTest2(args)
	SMessageBox(0,T "onEvtTest2", T "msgbox", 1);
	return 1;
end

function onEvtTstClick(args)
	local txt3=SStringW(L"append",-1);
	local sender=toSWindow(args.sender);
	sender:GetParent():CreateChildrenFromString(L"<button pos=\"0,0,150,30\" class=\"normalbtn\" id=\"12321\" show=\"1\" on_command=\"onEvtTest2\">lua btn 中文</button>");
	sender:SetVisible(0,1);
	return 1;
end
