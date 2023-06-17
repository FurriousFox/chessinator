const { Fen } = require("chess-fen");
let illegs = [];
let perftdiv = async function (depth, position, moves = []) {
    // console.log(position);
    let fenBoard = new Fen(position);
    for (let move of moves) {
        fenBoard = fenBoard.move(move);
    }
    console.log(fenBoard.toString());

    const { spawnSync, spawn } = require('child_process');
    const fs = require("fs");

    // run stockfish perftdiv
    let b = spawn("stockfish.exe");
    b.stdin.write(`position fen ${fenBoard.toString()}\ngo perft ${depth}\n`);
    b.stdin.end();
    let c = "";
    b.stdout.on("data", data => c += data);

    // run main.cpp perftdiv
    let a = spawnSync("main.exe", ["--perftdiv", depth, fenBoard.toString()]);
    perftC = Object.fromEntries(a.stdout.toString().split("\r\n").filter(e => e.includes(":")).map(e => [e.split(": ")[0], e.split(": ")[1]]));

    // wait for stockfish to finish
    await new Promise(resolve => b.on("close", resolve));
    perftS = Object.fromEntries(c.split("\r\n").filter(e => !e.includes("Nodes")).filter(e => e.includes(":")).map(e => [e.split(": ")[0], e.split(": ")[1]]));

    // console.log(c, "\n\n\n\n", a.stdout.toString());

    // check if all moves exist on both
    let d = Object.keys(perftC);
    let e = Object.keys(perftS);

    let i = d.filter(a => !e.includes(a));
    let g = e.filter(a => !d.includes(a));

    let f = [];
    for (let k of i) {
        switch (k) {
            case "e1h1":
                if (!g.includes("e1g1")) f.push("e1g1");
                else {
                    delete e[e.indexOf("e1g1")];

                    perftC.e1g1 = perftC.e1h1;
                    delete perftC.e1h1;
                }
                break;
            case "e1a1":
                if (!g.includes("e1c1")) f.push("e1c1");
                else {
                    delete e[e.indexOf("e1c1")];

                    perftC.e1c1 = perftC.e1a1;
                    delete perftC.e1a1;
                }
                break;
            case "e8h8":
                if (!g.includes("e8g8")) f.push("e8g8");
                else {
                    delete e[e.indexOf("e8g8")];

                    perftC.e8g8 = perftC.e8h8;
                    delete perftC.e8h8;
                }
                break;
            case "e8a8":
                if (!g.includes("e8c8")) f.push("e8c8");
                else {
                    delete e[e.indexOf("e8c8")];

                    perftC.e8c8 = perftC.e8a8;
                    delete perftC.e8a8;
                }
                break;
            default:
                f.push(k);
        }
    }
    g = e.filter(a => !d.includes(a));


    if (f.length > 0) {
        console.error(`illegal move:`, f);
        f.forEach(f => illegs.push(['illegal', moves, f, fenBoard.toString()]));
        delete perftC[f];
    }
    if (g.length > 0) {
        console.error(`missing move:`, g);
        g.forEach(f => illegs.push(['missing', moves, f, fenBoard.toString()]));
        delete perftS[g];
    }

    fs.writeFileSync("illegs.json", JSON.stringify(illegs, null, 4));

    // make sure all moves have the same amount of nodes on both
    let h = d.filter(a => perftC[a] != perftS[a]);
    console.log(h);
    for (let k of h) {
        let cloneofmoves = [...moves];
        cloneofmoves.push(k);
        await perftdiv(depth - 1, position, cloneofmoves);
        // break;
    }
};

let depth = 6;
let position = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
perftdiv(depth, position);