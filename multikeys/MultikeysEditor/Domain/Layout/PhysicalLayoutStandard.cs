namespace MultikeysEditor.Domain.Layout
{
    public enum PhysicalLayoutStandard
    {
        ANSI = 0,        // US layout, used for some other layouts
        ISO = 1,         // European layout, used for multiple layouts internationally
        ABNT_2 = 2,      // Brazilian portuguese layout
        JIS = 3,         // Japanese layout
        DUBEOLSIK = 4,   // Korean layout

        // Layouts with big return:
        ANSI_BIGRETURN = 64,
        ISO_BIGRETURN = 65,
        ABNT_2_BIGRETURN = 66,
        DUBEOLSIK_BIGRETURN = 67,
    }
}
