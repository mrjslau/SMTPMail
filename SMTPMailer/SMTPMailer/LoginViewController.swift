//
//  LoginViewController.swift
//  SMTPMailer
//
//  Created by Marijus Laucevicius on 21/05/2019.
//  Copyright © 2019 Marijus Laucevicius. All rights reserved.
//

import Cocoa

class LoginViewController: NSViewController {
    
    @IBOutlet var emailField: NSTextField!
    @IBOutlet var passwordField: NSSecureTextField!
    @IBOutlet var emailLabel: NSTextField!
    @IBOutlet var passwordLabel: NSTextField!
    
    //let confUrl = FileManager.default.homeDirectoryForCurrentUser.appendingPathComponent("Desktop/App.config")
    var username = ""
    var pass = ""

    override func viewDidLoad() {
        super.viewDidLoad()
        
        let userPath = FileManager.default.urls(for: .desktopDirectory, in: .userDomainMask)
        var userPathStr = userPath[0].absoluteString
        userPathStr.removeSubrange(userPathStr.startIndex..<userPathStr.index(userPathStr.startIndex, offsetBy: 7))
        
        var appConfPath = userPathStr
        appConfPath.append("SMTPconf/App.config")
        
        
        guard let aStreamReader = StreamReader(path: appConfPath) else { return }
        for line in aStreamReader {
            var lineArr = line.components(separatedBy: ":")
            if lineArr[0] == "USER" {
                username = lineArr[1]
            }
            else if lineArr[0] == "PASS" {
                pass = lineArr[1]
            }
        }
    }
    
    @IBAction func login(_ sender: Any) {
        if emailField.stringValue == username && passwordField.stringValue == pass {
            performSegue(withIdentifier: "loginToMainSegue", sender: Any?.self)
            emailLabel.textColor = .white
            passwordLabel.textColor = .white
            //TODO Dismiss
        }
        else {
            emailLabel.textColor = .red
            passwordLabel.textColor = .red
        }
    }
}
