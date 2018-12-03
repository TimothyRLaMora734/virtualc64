//
// This file is part of VirtualC64 - A user-friendly Commodore 64 emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
//

import Foundation

// The document of the currently active emulator instance
private var currentDocument: MyDocument? {
    get {
        return NSApplication.shared.orderedDocuments.first as? MyDocument
    }
}

// The controller of the currently active emulator instance
private var currentController: MyController? {
    get {
        return currentDocument?.windowControllers.first as? MyController
    }
}

// The emulator proxy of the currently active emulator instance
private var currentProxy: C64Proxy? {
    get {
        return currentDocument?.c64
    }
}

//
// Commands
//

func resetScriptCmd(arguments: [AnyHashable : Any]?) -> Bool {
    
    currentProxy?.powerUp()
    return true
}

func configureScriptCmd(arguments: [AnyHashable : Any]?) -> Bool {
    
    // Hardware model
    if let argument = arguments?["VC64HwModel"] as? String {
        switch argument {
        case "C64_PAL": currentProxy?.setModel(Int(C64_PAL.rawValue))
        case "C64_II_PAL": currentProxy?.setModel(Int(C64_II_PAL.rawValue))
        case "C64_OLD_PAL": currentProxy?.setModel(Int(C64_OLD_PAL.rawValue))
        case "C64_NTSC": currentProxy?.setModel(Int(C64_NTSC.rawValue))
        case "C64_II_NTSC": currentProxy?.setModel(Int(C64_II_NTSC.rawValue))
        case "C64_OLD_NTSC": currentProxy?.setModel(Int(C64_OLD_NTSC.rawValue))
        default: return false
        }
    }
    
    // Auto warp
    if let argument = arguments?["VC64AutoWarp"] as? String {
        track();
        switch argument {
        case "on": track(); currentProxy?.setWarpLoad(true)
        case "off": track(); currentProxy?.setWarpLoad(false)
        default: return false
        }
    }
    
    // Always warp
    if let argument = arguments?["VC64AlwaysWarp"] as? String {
        track();
        switch argument {
        case "on": track(); currentProxy?.setAlwaysWarp(true)
        case "off": track(); currentProxy?.setAlwaysWarp(false)
        default: return false
        }
    }
    
    // Auto mount
    if let argument = arguments?["VC64AutoMount"] as? String {
        track();
        switch argument {
        case "on": track(); currentController?.autoMount = true
        case "off": track(); currentController?.autoMount = false
        default: return false
        }
    }
    
    return true
}

func dragInScriptCmd(arguments: [AnyHashable : Any]?) -> Bool {
    
    if let argument = arguments?["VC64Path"] as? String {
        let url = URL(fileURLWithPath: argument)
        do {
            try currentDocument?.createAttachment(from: url)
            return currentDocument?.processAttachmentAfterDragAndDrop() ?? false
        } catch {
            track("Remote control: Emulated drag operation failed.")
        }
    }
    return false
}

func typeTextCmd(arguments: [AnyHashable : Any]?) -> Bool {
    
    if let text = arguments?[""] as? String {
        currentController?.keyboardcontroller.type(text)
        return true
    }
    return false
}

func takeScreenshotCmd(arguments: [AnyHashable : Any]?) -> Bool {
    
    var url: URL?
    var image: NSImage?
    var data: Data?
    
    // Compute URL
    if let path = arguments?["VC64Path"] as? String {
        url = URL(fileURLWithPath: path)
    }
    
    // Take screenshot
    image = currentController?.metalScreen.screenshot(afterUpscaling: false)
    if let format = arguments?["VC64ImageFormat"] as? String {
        switch format {
        case "tiff":
            data = image?.tiffRepresentation
        case "jpg":
            data = image?.jpgRepresentation
        case "png":
            data = image?.pngRepresentation
        default:
            break
        }
    } else {
        data = image?.pngRepresentation
    }
    
    // Write screenshot to URL
    if url != nil && data != nil {
        do {
            try data!.write(to: url!, options: .atomic)
            return true
        } catch {
            track("Remote control: Failed to save screenshot")
        }
    }
    return false
}
