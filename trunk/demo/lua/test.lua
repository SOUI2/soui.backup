function onEvtTest2(sender)
	SMessageBox(0,L "onEvtTest2", L "msgbox", 1);
	return 1;
end

function onEvtTstClick(args)
	--不支持在一个代码行中连续调用如A():B():C()
	local sender=args.sender;
	local luabtn=sender:GetParent():CreateChildrenFromString(L"<button pos=\"0,0,150,30\" class=\"normalbtn\" id=\"12321\" show=\"1\">lua btn 中文</button>");
	local scriptmod=theApp():GetScriptModule();
	local txt=luabtn:GetWindowText();
	local txt2=txt:Left(5);
	scriptmod:subscribeEvent(luabtn,10000,"onEvtTest2");
	sender:SetVisible(0,1);
	return 1;
end
