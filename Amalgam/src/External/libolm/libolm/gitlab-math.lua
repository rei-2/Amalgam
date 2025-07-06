function Math(el)
    if el.mathtype == "InlineMath" then
        if el.text:sub(1,1) == '`' and el.text:sub(#el.text) == '`' then
            local text = el.text:sub(2,#el.text-1)
            return pandoc.Math(el.mathtype, text)
        else
            local cont = pandoc.read(el.text)
            return { pandoc.Str("$") } .. cont.blocks[1].content .. { pandoc.Str("$") }
        end
    end
end

function CodeBlock(el)
    if el.classes[1] == "math" then
        return pandoc.Para({ pandoc.Math("DisplayMath", el.text) })
    end
end