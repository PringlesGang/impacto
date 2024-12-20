root.BacklogMenu = {
    Type = BacklogMenuType.CC,
    DrawType = DrawComponentType.SystemMenu,

    BacklogBackgroundSprite = "BacklogBackground",
    BacklogBackgroundRepeatHeight = 1005,

    BacklogHeaderSprite = "BacklogHeader",
    BacklogHeaderPosition = { X = 0, Y = 0 },

    BacklogControlsSprite = "BacklogControls",
    BacklogControlsPosition = { X = 0, Y = 986 },

    EntryHighlightSprite = "EntryHighlight",
    EntryHighlightLocation = EntryHighlightLocationType.TopLineLeftOfScreen,
    EntryHighlightOffset = { X = 0, Y = -3 },

    VoiceIconSprite = "VoiceIcon",
    VoiceIconOffset = { X = -16, Y = 0 },

    ScrollbarTrackSprite = "ScrollbarTrack",
    ScrollbarThumbSprite = "ScrollbarThumb",
    ScrollbarPosition = { X = 1656, Y = 40 },
    ScrollbarThumbLength = 87,

    EntriesStart = { X = 547, Y = 149 },
    RenderingBounds = { X = 194, Y = 121, Width = 1531, Height = 868 },
    EntryYPadding = 26,

    FadeInDuration = 0.5,
    FadeOutDuration = 0.5,
    FadeInDirectDuration = 0.25,
    FadeOutDirectDuration = 0.25,

    ScrollingSpeed = 900,
    MinHoldTime = 0.5,
    AdvanceFocusTimeInterval = 0.05,
    PageUpDownHeight = 765,

    MenuMask = "MenuMask",
    BacklogMask = "BacklogMask",
    HoverBounds = { X = 380, Y = 145, Width = 1230, Height = 820 }
};

root.Sprites["BacklogBackground"] = {
    Sheet = "Backlog",
    Bounds = { X = 0, Y = 0, Width = 1920, Height = 1080 },
};

root.Sprites["BacklogHeader"] = {
    Sheet = "Backlog",
    Bounds = { X = 962, Y = 1210, Width = 542, Height = 542 },
};

root.SpriteSheets["BacklogMask"] = {
    Path = { Mount = "system", Id = 2 },
    DesignWidth = 1920,
    DesignHeight = 1080
};

root.Sprites["BacklogControls"] = {
    Sheet = "Backlog",
    Bounds = { X = 0, Y = 1151, Width = 1920, Height = 59 },
}

root.Sprites["MenuMask"] = {
    Sheet = "MenuChip",
    Bounds = { X = 154, Y = 140, Width = 1900, Height = 1061 },
};

root.Sprites["VoiceIcon"] = {
    Sheet = "Backlog",
    Bounds = { X = 1521, Y = 1218, Width = 38, Height = 38 },
};

root.Sprites["EntryHighlight"] = {
    Sheet = "Backlog",
    Bounds = { X = 0, Y = 1105, Width = 1920, Height = 44 }
};

root.Sprites["ScrollbarThumb"] = {
    Sheet = "Backlog",
    Bounds = { X = 2027, Y = 1115, Width = 18, Height = 83 },
};

root.Sprites["ScrollbarTrack"] = {
    Sheet = "Backlog",
    Bounds = { X = 2030, Y = 41, Width = 11, Height = 915 },
};