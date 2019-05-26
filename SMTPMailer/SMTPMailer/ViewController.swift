//
//  ViewController.swift
//  SMTPMailer
//
//  Created by Marijus Laucevicius on 21/05/2019.
//  Copyright © 2019 Marijus Laucevicius. All rights reserved.
//

import Cocoa

class ViewController: NSViewController {

    @IBOutlet var sendEmailButton: NSButton!
    @IBOutlet var sendEmailSuccessLabel: NSTextField!
    
    @IBOutlet var recipientLabel: NSTextField!
    @IBOutlet var subjectLabel: NSTextField!
    @IBOutlet var contentLabel: NSTextField!
    
    @IBOutlet var recipientTextField: NSTextField!
    @IBOutlet var subjectTextField: NSTextField!
    @IBOutlet var contentTextField: NSTextField!
    
    @IBAction func sendEmail(_ sender: Any) {
        
        if ((recipientTextField?.stringValue) != "") && ((subjectTextField?.stringValue) != "") && ((contentTextField?.stringValue) != "") {
            
            recipientLabel.textColor = .white
            subjectLabel.textColor = .white
            contentLabel.textColor = .white
            
            let writeFilename = FileManager.default.homeDirectoryForCurrentUser.appendingPathComponent("Desktop/SMTPconf/App.config")
            var string = ""
            
            let userPath = FileManager.default.urls(for: .desktopDirectory, in: .userDomainMask)
            var userPathStr = userPath[0].absoluteString
            userPathStr.removeSubrange(userPathStr.startIndex..<userPathStr.index(userPathStr.startIndex, offsetBy: 7))
            
            var appTextPath = userPathStr
            appTextPath.append("SMTPconf/App.text")
            var runscriptPath = userPathStr
            runscriptPath.append("SMTPMailer/SMTPMailer/runscript.command")
            
            guard let aStreamReader = StreamReader(path: appTextPath) else { return }
            for line in aStreamReader {
                var lineArr = line.components(separatedBy: ":")
                if lineArr[0] == "RCPT" {
                    var strtmp = "RCPT:"
                    strtmp.append(recipientTextField!.stringValue)
                    string.append(strtmp)
                    string.append("\n")
                }
                else if lineArr[0] == "SUBJ" {
                    var strtmp = "SUBJ:"
                    strtmp.append(subjectTextField!.stringValue)
                    string.append(strtmp)
                    string.append("\n")
                }
                else if lineArr[0] == "CONT" {
                    var strtmp = "CONT:"
                    strtmp.append(contentTextField!.stringValue)
                    string.append(strtmp)
                    string.append("\n")
                }
                else {
                    string.append(line)
                    string.append("\n")
                }
            }
            do {
                try string.write(to: writeFilename, atomically: true, encoding: String.Encoding.utf8)
            } catch {}
            
            let task = Process()
            task.launchPath = "/bin/bash"
            task.arguments = [runscriptPath]
            let pipe = Pipe()
            task.standardOutput = pipe
            task.standardError = pipe
            
            task.launch()
            task.waitUntilExit()
            let data = pipe.fileHandleForReading.readDataToEndOfFile()
            let output: String = NSString(data: data, encoding: String.Encoding.utf8.rawValue)! as String
            print(output)
            
        } else {
            recipientLabel.textColor = .red
            subjectLabel.textColor = .red
            contentLabel.textColor = .red
        }
        
    }
}

