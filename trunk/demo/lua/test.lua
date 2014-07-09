function onEvtTest2(sender,nmhdr)
	SMessageBox(0,L "onEvtTest2", L "msgbox", 1);
	return 1;
end

function onEvtTstClick(args)
	--不支持在一个代码行中连续调用如A():B():C()
	local sender=args.sender;
	local luabtn=sender:GetParent():CreateChildrenFromString(L"<button pos=\"0,0,100,30\" class=\"normalbtn\" id=\"12321\" show=\"1\">lua btn</button>");
	local sapp=SApplication_getSingleton();
	local scriptmod=sapp:GetScriptModule();
	scriptmod:subscribeEvent(luabtn,10000,"onEvtTest2");
	sender:SetVisible(0,1);
	return 1;
end
