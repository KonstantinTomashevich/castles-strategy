UIResizer = ScriptObject ()

function UIResizer:Start ()
    self.continiousUpdate = false
    self.scanRootElement = "UIRoot"

    self.lastWidth = 0
    self.lastHeight = 0
end

function UIResizer:Update ()
    if self.continiousUpdate or self.lastWidth ~= graphics.width or
        self.lastHeight ~= graphics.height then

        self.lastWidth = graphics.width
        self.lastHeight = graphics.height

        local scanRootElement = nil
        if self.scanRootElement == "UIRoot" then
            scanRootElement = ui.root
        else
            scanRootElement = ui.root:GetChild (self.scanRootElement, true)
        end

        if scanRootElement ~= nil then
            self:ProcessElement (scanRootElement)
        end
    end
end

function UIResizer:ProcessElement (element)
    if element:HasTag ("UIResizer") then
        local elementVars = element:GetVars ()
        local vWidth = elementVars [StringHash ("VWidth")]:GetFloat ()
        local vHeight = elementVars [StringHash ("VHeight")]:GetFloat ()

        local vX = elementVars [StringHash ("VX")]:GetFloat ()
        local vY = elementVars [StringHash ("VY")]:GetFloat ()

        local wDep = self:ReadDep (element, "WDep", elementVars)
        local hDep = self:ReadDep (element, "HDep", elementVars)
        local xDep = self:ReadDep (element, "XDep", elementVars)
        local yDep = self:ReadDep (element, "YDep", elementVars)

        element:SetSize (math.floor (vWidth * wDep), math.floor (vHeight * hDep))
        element:SetPosition (math.floor (vX * xDep), math.floor (vY * yDep))

        if element.type == StringHash ("Label") or
            element.type == StringHash ("LineEdit") then

            local vTextSize = elementVars [StringHash ("VTextSize")]:GetFloat ()
            local tsDep = self:ReadDep (element, "TSDep", elementVars)

            if element.type == StringHash ("Label") then
                element.textSize = math.floor (vTextSize * tsDep)
            else
                element.textElement.textSize = math.floor (vTextSize * tsDep)
            end
        end

        local children = element:GetChildrenWithTag ("UIResizer")
        for index, child in pairs (children) do
            self:ProcessElement (child)
        end

        local layoutTypeVariant = elementVars [StringHash ("layoutType")]
        local layoutType = nil
        if layoutTypeVariant ~= nil then
            layoutType = layoutTypeVariant:GetString ()
        end

        if layoutType == "V" or layoutType == "H" then
            local updatePrimarySize = elementVars ["updatePS"]:GetBool ()
            local updateSecondarySize = elementVars ["updateSS"]:GetBool ()

            local spacing = nil
            if layoutType == "V" then
                spacing = element.layoutSpacing * hDep / 1000.0
            else
                spacing = element.layoutSpacing * wDep / 1000.0
            end

            local currentX = element.layoutBorder.left * wDep / 1000.0
            local currentY = element.layoutBorder.top * hDep / 1000.0

            for index, child in pairs (children) do
                child:SetPosition (math.floor (currentX), math.floor (currentY))

                if layoutType == "V" then
                    currentY = currentY + child.height + spacing
                else
                    currentX = currentX + child.width + spacing
                end
            end

            if updatePrimarySize then
                if layoutType == "V" then
                    element.height = math.floor (currentY - spacing +
                        element.layoutBorder.bottom * hDep / 1000.0)

                else
                    element.width = math.floor (currentX - spacing +
                        element.layoutBorder.right * wDep / 1000.0)
                end
            end

            if updateSecondarySize then
                local maxSecondarySize = 0

                for index, child in pairs (children) do
                    local secondary = nil
                    if layoutType == "V" then
                        secondary = child.width
                    else
                        secondary = child.height
                    end

                    if secondary > maxSecondarySize then
                        maxSecondarySize = secondary
                    end
                end

                if layoutType == "V" then
                    element.width = math.floor (currentX + maxSecondarySize +
                        element.layoutBorder.right * wDep / 1000.0)
                else
                    element.height = math.floor (currentY + maxSecondarySize +
                        element.layoutBorder.bottom * hDep / 1000.0)
                end
            end
        end
    end
end

function UIResizer:ReadDep (element, name, elementVars)
    local var = elementVars [StringHash (name)]:GetString ()
    if var:sub (1, 1) == "P" then
        if var:sub (2, 2) == "W" then
            return element.parent.width
        else
            return element.parent.height
        end
    else
        if var:sub (2, 2) == "W" then
            return graphics.width
        else
            return graphics.height
        end
    end
end
