CREATE TABLE IF NOT EXISTS intrebari (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    intrebare TEXT NOT NULL,
    raspuns1 TEXT NOT NULL,
    raspuns2 TEXT NOT NULL,
    raspuns3 TEXT NOT NULL,
    raspuns_corect TEXT NOT NULL
);


INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Cine a fost Grigore Moisil?', 'Doctor', 'Cantaret','Profesor', 'Profesor');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('In ce  an s-a terminat cel de-al doilea razboi mondial?', '1939', '1918', '1945', '1945');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Cine a fost Alexandru Ioan Cuza?', 'Doctor', 'Cantaret','Inteiemetorul Romaniei', 'Inteiemetorul Romaniei');

INSERT INTO intrebari(intrebare, raspuns1, raspuns2, raspuns3, raspuns_corect) VALUES 
    ('Ce este capitala Frantei?', 'Madrid', 'Berlin', 'Paris', 'Paris');