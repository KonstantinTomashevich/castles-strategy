class PrefabUpdater : ScriptObject
{
    void Update (float timeStep)
    {
        Component @village = node.parent.GetComponent ("Village");
        int ownership = village.GetAttribute ("Ownership").GetInt ();
        String text = Abs (ownership) + "LP";

        Text3D @infoText = node.GetChild ("info").GetComponent ("Text3D");
        infoText.text = text;
        if (ownership > 0)
        {
            infoText.color = Color (0.0f, 0.0f, 0.6f, 1.0f);
        }
        else
        {
            infoText.color = Color (0.6f, 0.0f, 0.0f, 1.0f);
        }
    }
}
