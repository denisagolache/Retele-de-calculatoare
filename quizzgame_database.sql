CREATE TABLE IF NOT EXISTS intrebari (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    intrebare VARCHAR(1024),
    raspuns1 VARCHAR(1024),
    raspuns2 VARCHAR(1024),
    raspuns3 VARCHAR(1024),
    raspuns_corect VARCHAR(1024)
    
);

CREATE TABLE IF NOT EXISTS intrebariFolosite(
id INTEGER PRIMARY KEY AUTOINCREMENT,
id_intrebare INTEGER NOT NULL,
id_user INTEGER NOT NULL,
used INTEGER DEFAULT 0,
FOREIGN KEY (id_intrebare) REFERENCES intrebari(id),
UNIQUE(id_user, id_intrebare)
);

CREATE TABLE IF NOT EXISTS users_1 (
    id_user INTEGER PRIMARY KEY AUTOINCREMENT,
    username VARCHAR(1024),
    score INTEGER DEFAULT 0
);

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Linux este: ', 'software', 'aplicatie','sistem de operare', 'sistem de operare');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Care este un protocol orientat-conexiune?', 'UDP', 'TCP','IPX/SPX', 'TCP');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Ce companie a dezvoltat prima data limbajul de programare Java?', 'Microsoft', 'IBM','Sun Microsystems', 'Sun Microsystems');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Sistemul binar utilizeaza puterea lui: ', '2', '4','8', '2');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Care dintre urmatoarele comenzi NU apartine bibliotecii string.h?', 'strcmp', 'pow', 'atoi', 'pow');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Care dintre urmatoarele este limbaj orientat-obiect?', 'JAVA', 'Assembly', 'HTML', 'JAVA');    

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Cine e fondatorul Facebook?', 'Bill Gates', 'Paul Allen', 'Mark Zuckerberg', 'Mark Zuckerberg');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Cine a fost inventatorul masinii TURING?', 'Alan Turing', 'Albert Einstein', 'Edsger Dijkstra', 'Alan Turing');
   
INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Care a fost numele primului calculatorul?', 'Turing', 'Eniac','Mac', 'Eniac');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Cine a fost Grigore Moisil?', 'Doctor', 'Cantaret','Profesor', 'Profesor');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('De la ce vine litera "S" din terminologia WEB "HTTPS"?', 'Safe', 'Secure', 'Short', 'Secure');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Cate coduri contine ASCII?', '128', '256', '64', '128');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Virusul este _______ de calculator ', 'program', 'fisier', 'baza de date', 'program');


INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Care este reprezentarea numarului 3 in binar?', '11', '10', '01', '11');


INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Cine a inventat limbajul de programare C?', 'Denis Ritchie', 'Albert Einstein', 'Edsger Dijkstra', 'Denis Ritchie');


INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Care este complexitatea, in medie, a QuickSort?', 'O(n*n)', 'O(n log n)', 'O(n)', 'O(n log n)');



INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Ce inseamna "DOS" in termeni informatici? ', 'DISK OPERATING SYSTEM', 'DOMAIN OPERATING SYSTEM', 'DEBIAN OPERATING SYSTEM', 'DISK OPERATING SYSTEM');


INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Cine a dezvoltat sistemul de operare LINUX?', 'Linus Torvalds', 'Bill Gates', 'Steve Jobs', 'Linus Torvalds');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('CE inseamna "API" in termeni informatici?', 'ARTIFICIAL PROGRAMMING INTELLIGENCE ', 'APLICATION PROGRAMMING INTERFACE', 'ANTIVIRUS PROGRAMMING INTERFACE', 'APLICATION PROGRAMMING INTERFACE');
