#include "MainForm.hpp"
#include "DefinitionParser.hpp"
#include "iPediaApplication.hpp"
#include "SimpleSocketConnection.hpp"
#include "SocketAddress.hpp"

void MainForm::resize(const RectangleType& screenBounds)
{
    setBounds(screenBounds);

    RectangleType bounds;
    UInt16 index=getObjectIndex(definitionScrollBar);
    getObjectBounds(index, bounds);
    bounds.topLeft.x=screenBounds.extent.x-8;
    bounds.extent.y=screenBounds.extent.y-36;
    setObjectBounds(index, bounds);
    
    index=getObjectIndex(termLabel);
    getObjectBounds(index, bounds);
    bounds.topLeft.y=screenBounds.extent.y-14;
    setObjectBounds(index, bounds);

    index=getObjectIndex(termInputField);
    getObjectBounds(index, bounds);
    bounds.topLeft.y=screenBounds.extent.y-14;
    bounds.extent.x=screenBounds.extent.x-60;
    setObjectBounds(index, bounds);

    index=getObjectIndex(goButton);
    getObjectBounds(index, bounds);
    bounds.topLeft.x=screenBounds.extent.x-26;
    bounds.topLeft.y=screenBounds.extent.y-14;
    setObjectBounds(index, bounds);

    update();    
}

void MainForm::draw(UInt16 updateCode)
{
    iPediaForm::draw(updateCode);
    ArsLexis::Rectangle rect=bounds();
    WinPaintLine(rect.x(), rect.height()-18, rect.width(), rect.height()-18);

    rect.explode(2, 18, -12, -36);

    const iPediaApplication& app=static_cast<iPediaApplication&>(application());
    definition_.render(rect, app.renderingPreferences());
    
    UInt16 index=getObjectIndex(definitionScrollBar);
    ScrollBarType* scrollBar=static_cast<ScrollBarType*>(FrmGetObjectPtr(*this, index));
    SclSetScrollBar(scrollBar, definition_.firstShownLine(), 0, definition_.totalLinesCount()-definition_.shownLinesCount(), definition_.shownLinesCount());
}


Err MainForm::initialize()
{
    Err error=iPediaForm::initialize();
    if (!error)
    {
        ArsLexis::String text(            
"[[af:Wetenskapsfiksie]] [[da:Science-fiction]] [[de:Science-Fiction]] [[eo:Sciencfikcio]] [[es:Ciencia ficción]] [[fi:Tieteiskirjallisuus]] [[fr:Science-fiction]] [[he:&#1502;&#1491;&#1506; &#1489;&#1491;&#1497;&#1493;&#1504;&#1497;]][[ja:&#12469;&#12452;&#12456;&#12531;&#12473;&#12539;&#12501;&#12451;&#12463;&#12471;&#12519;&#12531;]] [[nl:Science fiction]] [[no:Science fiction]] [[pl:Science fiction]] [[sv:Science fiction]][[ru:&#1053;&#1072;&#1091;&#1095;&#1085;&#1072;&#1103; &#1092;&#1072;&#1085;&#1090;&#1072;&#1089;&#1090;&#1080;&#1082;&#1072;]]\n"
"\n"
"'''Science fiction''' is a form of [[fiction]] which deals principally with the impact of actual or imagined science (and/or technology) upon society or individuals. \n"
"\n"
"==Scope==\n"
"Sometimes the characters involved are not even human, but are imagined aliens or other products of Earth evolution. The term is more generally used to refer to any literary fantasy that includes a [[Science|scientific]] factor as an essential orienting component, and even more generally used to refer to any [[fantasy]] at all. Such literature may consist of a careful and informed extrapolation of scientific facts and principles, or it may range into far-fetched areas flatly contradictory of such facts and principles. In either case, plausibility based on science is a requisite, so that such precursors of the genre as [[Mary Wollstonecraft Shelley]]'s Gothic novel ''[[Frankenstein]], or the Modern Prometheus'' ([[1818]]) and [[Robert Louis Stevenson]]'s ''Strange Case of Dr. Jekyll and Mr. Hyde'' ([[1886]]) are plainly science fiction, whereas [[Bram Stoker]]'s ''[[Dracula]]'' ([[1897]]), based purely on the [[Supernatural]], is not.\n"
"\n"
"==Types of science fiction==\n"
"\n"
"===Hard science fiction===\n"
"''Main article'': [[Hard science fiction]]\n"
"\n"
"Hard science fiction, or hard SF, is a subgenre of science fiction characterized by an interest in [[Science|scientific]] detail or accuracy.  Hard SF stories focus on the natural sciences and technological developments.  Some authors scrupulously eschew such implausibilities as faster-than-light travel, while others accept such plot devices but nonetheless show a concern with a realistic depiction of the worlds that such a technology might make accessible. Character development is sometimes secondary to explorations of [[astronomy|astronomical]] or [[physics|physical]] phenomena, but other times authors make the human condition forefront in the story. However a common theme of hard SF has the resolution of the plot often hinging upon a technological point.  Writers attempt to have their stories consistent with known science at the time of publication.\n"
"\n"
"===Soft science fiction===\n"
"''Main article'': [[Soft science fiction]]\n"
"\n"
"[[Soft science fiction]] is science fiction whose plots and themes tend to focus on [[philosophy]], [[psychology]], [[politics]] and [[sociology]] while de-emphasizing the details of technological hardware and physical laws.  It is so-called 'soft' science fiction, because these subjects are grouped together as the soft sciences or humanities. For instance, in ''[[Dune (novel)|Dune]]'', [[Frank Herbert]] uses the plot device of a universe which has rejected conscious machines and has reverted to a feudal society. Consequently Herbert uses the Dune saga to comment about the human condition and make direct parallels to current socio-political realities. Soft science fiction may explore the reactions of societies or individuals to problems posed by natural phenomena or technological developments, but the technology will be a means to an end, not an end itself.\n"
"\n"
"===Other types===\n"
"There are, of course, many borderline cases of works using outer-space settings and futuristic-looking technology as little more than window-dressing for tales of adventure, romance, and other typical dramatic themes; examples include [[Star Wars]] (which is considered by some diehards to be not science fiction but [[fantasy]]) and many Hollywood [[space opera]]s. Some fans of [[hard science fiction]] would regard such films as fantasy, whereas the general public would probably place them squarely in the science fiction category. It has been suggested as a method of resolving this confusion that SF come to stand for [[speculative fiction]] and thus encompass [[fantasy]], [[horror fiction]], and [[sci-fi]] genres.\n"
"\n"
"==History of science fiction==\n"
"\n"
"===Forerunners of science fiction===\n"
"Science fiction was made possible only by the rise of modern science itself, notably the revolutions in astronomy and physics.  Aside from the age-old genre of fantasy literature, which does not qualify, there were notable precursors: \n"
"imaginary voyages to the moon in the 17th century, first shown in [[Johannes Kepler]]'s ''[[Somnium]]'' (The Dream, [[1634]]), then in [[Cyrano de Bergerac]]'s ''[[Comical History of the States and Empires of the Moon]]''  ([[1656]]), space travel in [[Voltaire]]'s ''[[Micromégas]]'' ([[1752]]), alien cultures in [[Jonathan Swift]]'s ''[[Gulliver's Travels]]'' ([[1726]]), and science fiction elements in the 19th-century stories of [[Edgar Allan Poe]], [[Nathaniel Hawthorne]], and [[Fitz-James O'Brien]]. In [[Romantic Poetry]], too, the writers' imaginations leapt to visions of other worlds and distant futures as in [[Alfred Lord Tennyson]]'s '[[Locksley Hall]]'.\n"
"\n"
"===Early science fiction===\n"
"\n"
"The European brand of science fiction proper began, however, toward the end of the 19th century with the scientific romances of [[Jules Verne]], whose science was rather on the level of invention, as well as the science-oriented novels of social criticism by [[H.G. Wells]].  Although better known for other works, [[Sir Arthur Conan Doyle]] also wrote early science fiction.\n"
"\n"
"The development of American science fiction as a self-conscious [[genre]] dates (in part) from [[1926]], when [[Hugo Gernsback]] founded ''[[Amazing Stories]]'' magazine, which was devoted exclusively to science fiction stories. Since he is notable for having chosen the variant term ''scientifiction'' to describe this incipient genre, the stage in the genre's development, his name and the term \"scientifiction\" are often thought to be inextricably linked. Published in this and other [[Pulp magazine|pulp magazines]] with great and growing success, such scientifiction stories were not viewed as serious literature but as sensationalism.\n"
"\n"
"===The Golden Age===\n"
"''Main Article'' : [[Astounding Magazine]]\n"
"\n"
"With the advent in [[1937]] of a demanding editor, [[John W. Campbell|John W. Campbell, Jr.]], of ''[[Analog Science Fiction|Astounding Science Fiction]]'' (founded in [[1930]]), and with the publication of stories and novels by such writers as [[Isaac Asimov]], [[Arthur C. Clarke]], and [[Robert Heinlein]], science fiction emerged as a mode of serious fiction.  \n"
"Ventures into the genre by writers who were not devoted exclusively to science fiction, such as [[Karel Capek]], [[Aldous Huxley]], [[C. S. Lewis]] and, later, [[Ray Bradbury]] and [[Kurt Vonnegut]] Jr., also added respectability. Magazine covers of bug-eyed monsters and scantily-clad women preserved the sensational image for many, however.\n"
"\n"
"===The post-war era===\n"
"A great boom in the popularity of science fiction followed [[World War II]].  \n"
"Some science fiction works became paperback best-sellers. \n"
"\n"
"===The modern era===\n"
"The modern era began in the  mid 1960's with the popularisation of the genre of [[soft science fiction]]. In literary terms it dates roughly from the publication of [[Frank Herbert]]'s ''[[Dune (novel)|Dune]]'' in 1965, a dense, complex, and detailed work of fiction featuring political intrigue in a future galaxy, strange and mystical religious beliefs, and the eco-system of the desert planet [[Arrakis]]. While in 1966 [[Gene Roddenberry]]'s ''[[Star Trek]]'' brought such science fiction to a mass television audience. The original ''Star Trek'' seems out of date today, but at the time it was at the forefront of [[liberalism]]. It preached the universality and equality of humanity. It had an attractive black officer, the first black-white kiss on American TV, a Russian officer at the height of the Cold War, an Asian officer, and even an alien officer.\n"
"\n"
"The field saw an increase in:\n"
"*the number of writers and readers \n"
"*the breadth of subject matter \n"
"*the depth of treatment \n"
"*the sophistication of language and technique \n"
"*the political and literary consciousness of the writing. \n"
"\n"
"Also, technological fixes to a problem became a far rarer plot device.\n"
"\n"
"A second generation of original and popular science fiction films begin to appear, among the most significant of which were ''[[2001: A Space Odyssey]]'' ([[1968]]), ''[[THX 1138]]'' ([[1969]]) ''[[Close Encounters of the Third Kind]]'', ([[1977]]), and ''[[Star Wars]]'', ([[1977]]). (See the [[list of science fiction films]] article for a more detailed list of notable science fiction films).\n"
"\n"
"The success of ''[[Star Wars]]'' was especially influential since it caused an explosive increase of interest in science fiction (in all forms) for several years after its release (though this has since abated, somewhat). Science fiction literature strongly benefitted from this heightened interest and science fiction or fantasy titles frequently filled the bestseller lists well into the [[1980s]].  Eventually, cultural interest in science fiction literature declined somewhat with consumer fatigue, flooded markets, and competition from other entertainment venues being a few of the reasons for this. Also, science fictional or fantasy \"elements\" began to be usurped by traditional authors and other types of media, though they were not significant enough to be classified as purely science fiction or fantasy. Today, ''pure'' science fiction or fantasy books only occasionally make the bestseller lists, although, in overall numbers there are more science fiction or fantasy books published now than in the past. Science fiction literature magazines, on the other hand, have seen a progressive and steady decline over the last 50 years.\n"
"\n"
"The influence of fantasy on the genre resulted in what is now called science fantasy. Contributions of these works to the literature of the fantastic include an awareness of irrationality and the inexplicable, the transformative force of language, and the power of myth to organize experience. ''[[Star Wars]]'' ([[1977]]) is the most powerful example of this trend.\n"
"\n"
"The increasing intellectual sophistication of the genre and the emphasis on wider societal and psychological issues significantly broadened the appeal of science fiction to the reading public. Science fiction became international, extending into the then [[Soviet Union]] and other eastern European nations, where it was frequently used as a vehicle for political commentary that could not be safely published in other forms. The Polish author [[Stanislaw Lem]] is one of the non-English science fiction writers who has become widely known outside his native country. Serious criticism of the genre is now common, and science fiction is studied in colleges and universities, both as literature and in how it relates to science and society.\n"
"\n"
"The principal science fiction awards are the [[Hugo_Award|Hugo]] and [[Nebula_Award|Nebula]].\n"
"\n"
"Science fiction has also been popular in [[radio]], [[comic books]], [[television]], and [[movies]]; it is notable that about three-quarters of the [[top twenty highest grossing films]] (source: IMDb June 2002) are based around science-fiction or fantasy themes.\n"
"\n"
"== Fandom ==\n"
"One of the unique features of the science fiction genre is its strong fan community, of which many authors are a firm part. Many people interested in science-fiction wish to interact with others who share the same interests; over time an entire culture of [[science fiction fandom]] has evolved. Local fan groups exist in most of the English-speaking world, as well as in Japan, Europe, and elsewhere; these groups often publish their own works. Also, fans were the originator of [[science fiction convention]]s, which gave them a way of getting together to discuss their mutual interest.  The original and largest convention is the [[Worldcon]].\n"
"\n"
"Many  [[fanzine]]s (\"fan magazines\") (and a few professional ones) exist that are dedicated solely to informing the science fiction fan on all aspects of the genre.  \n"
"The premiere awards of science fiction, the [[Hugo Award]]s, are awarded by members of the annual [[Worldcon]], which is almost entirely volunteer-run by fans.\n"
"\n"
"Science fiction fandom often overlaps with other similar interests, such as fantasy, [[role playing games]] and the [[Society for Creative Anachronism]].\n"
"\n"
"== Genres and subcategories ==\n"
"*[[Hard science fiction]]\n"
"*[[Soft science fiction]]\n"
"*[[Space opera]]\n"
"*[[Military science fiction]]\n"
"*[[Science fantasy]]\n"
"*[[Cyberpunk]]\n"
"*[[Apocalyptic and post-apocalyptic science fiction]]\n"
"*[[Postcyberpunk]]\n"
"*[[Social fiction]]\n"
"*[[Clerical fiction]]\n"
"*[[Political fiction]]\n"
"*[[New Wave (science fiction)]]\n"
"*[[Alternate history]]\n"
"*[[Utopian and dystopian fiction]]\n"
"*[[Comic science fiction]]\n"
"*[[Science fiction sitcom]]\n"
"*[[Science fiction erotica]]\n"
"*[[Lesbian science fiction]]\n"
"*[[Space-rock]]\n"
"*[[Xenofiction]]\n"
"*[[Time travel]]\n"
"\n"
"== Related topics ==\n"
"*[[List of science fiction authors]]\n"
"*[[Women in science fiction]]\n"
"*[[Science fiction convention]]s\n"
"*[[Science fiction film]]\n"
"*[[Science fiction television]]\n"
"*[[List of science fiction visual artists]]\n"
"*[[Science fiction fandom]]\n"
"*[[List of science fiction awards]]\n"
"*[[Internet Speculative Fiction DataBase]]\n"
"*[[Fantasy]]\n"
"*[[Horror]]\n"
"*[[Science Fiction and Fantasy Writers of America]] \n"
"*[[Science fiction themes]]\n"
"*[[List of science fiction novels]]\n"
"*[[List of science fiction short stories]]\n"
"*[[List of science fiction television]]\n"
"*[[Sex in science fiction]]\n"
"*[[Computers in fiction]]\n"
"*[[Fictional technology]]\n"
"*[[Internet Speculative Fiction Database]]\n"
"*[[Planets in science fiction]]\n"
"*[[Baen Free Library]]\n"
"\n"
"== External links ==\n"
"*[http://www.seismicfish.com/page/page/893925.htm New epublisher of quality science fiction ezine (free), science fiction ejournal, articles and entertainment. Submissions welcome.] \n"
"*[http://isfdb.tamu.edu/ Internet Speculative Fiction Database]\n"
"* ''[[The Encyclopedia of Science Fiction]]'' by [[John Clute]] (Ed.) and [[Peter Nicholls]] (Ed.) ISBN 031213486X\n"
"* [http://www.nvcc.edu/home/ataormina/scifi/default.htm Excellent Science Fiction Guide]\n"
"* [http://wondersmith.com/scifi/index.htm 'Free Sci-Fi Classics' A website providing archives of full text, non-copyright science fiction by Voltaire, Mary Shelley, Mark Twain, Jules Verne, H.G. Wells, Rudyard Kipling and others]\n"
"* [http://www.litrix.com/sec8.htm Online Sci-Fi at Litrix reading room, Authors: Richard Jefferies, Edgar Rice Burroughs, H. G. Wells, George Allan England, Mary Wollstonecraft (Godwin) Shelley, Philip José Farmer, A. Merritt, Jules Verne, Robert Louis Stevenson, Mallory Clontz, David Lindsay]\n"
"* [http://www.infinityplus.co.uk/index.htm Infinity Plus: Online new works in sci-fi, fantasy and horror]\n"
"* [http://greatsfandf.com/ Great Science-Fiction & Fantasy Works]: an attempt to extract the more literate authors and works from the morass.\n"
    );        
        DefinitionParser parser;
        parser.parse(text, definition_);                
    }
    return error;
}

inline void MainForm::handleScrollRepeat(const sclRepeat& data)
{
    definition_.scroll(data.newValue-data.value);
}

void MainForm::handlePenUp(const EventType& event)
{
    PointType point;
    point.x=event.screenX;
    point.y=event.screenY;
    if (definition_.bounds() && point)
        definition_.hitTest(point); 
}

void MainForm::handleControlSelect(const ctlSelect& data)
{
    assert(data.controlID==goButton);
    iPediaApplication& app=static_cast<iPediaApplication&>(application());
    using namespace ArsLexis;
    SocketConnectionManager* manager=0;
    Err error=app.getConnectionManager(manager);
    if (!error)
    {
        assert(manager);
        SimpleSocketConnection* connection=new SimpleSocketConnection(*manager);
        connection->setTransferTimeout(app.ticksPerSecond()*15L);
        error=connection->open(INetSocketAddress(0xcf2c860b, 80), "GET / HTTP/1.0\r\n\r\n");
        if (!(errNone==error || netErrWouldBlock==error))
            delete connection;
    }
}


Boolean MainForm::handleEvent(EventType& event)
{
    Boolean handled=false;
    switch (event.eType)
    {
        case ctlSelectEvent:
            handleControlSelect(event.data.ctlSelect);
            break;
        
        case penUpEvent:
            handlePenUp(event);
            break;
            
        case sclRepeatEvent:
            handleScrollRepeat(event.data.sclRepeat);
            break;
    
        default:
            handled=iPediaForm::handleEvent(event);
    }
    return handled;
}