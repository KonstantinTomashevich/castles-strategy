class PrefabUpdater : ScriptObject
{
    private bool firstUpdate_ = true;

    void Update (float timeStep)
    {
        Component @unit = node.parent.GetComponent ("Unit");
        if (firstUpdate_)
        {
            StaticModel @model = node.GetChild ("model").GetComponent ("StaticModel");
            model.material = cache.GetResource ("Material", unit.GetAttribute ("Is Belongs To First").GetBool () ?
                "DefaultUnits/Materials/DefaultFirst.xml" : "DefaultUnits/Materials/DefaultSecond.xml");
            firstUpdate_ = false;
        }

        String text = unit.GetAttribute ("HP").GetUInt () + " HP";
        if (node.parent.vars ["IsSelected"].GetBool ())
        {
            text = "[" + text + "]";
        }

        Text3D @infoText = node.GetChild ("info").GetComponent ("Text3D");
        infoText.text = text;
    }
}
