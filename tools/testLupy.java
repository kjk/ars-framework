import org.apache.lucene.analysis.standard.StandardAnalyzer;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.document.Document;
import org.apache.lucene.document.Field;
import org.apache.lucene.document.DateField;

import java.io.Reader;
import java.io.FileInputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.File;
import java.util.Date;

class Article {
	private String _title;
	private String _body;

	public Article(String title, String body) {
		_title = title;
		_body = body;
	}
	public String getTitle() { return _title; }
	public String getBody() { return _body; }

	public Document Document() {

	  // make a new, empty document
	  Document doc = new Document();

	  // Add the path of the file as a field named "path".	Use a Text field, so
	  // that the index stores the path, and so that the path is searchable
	  doc.add(Field.Text("title", getTitle()));

	  // Add the last modified date of the file a field named "modified".  Use a
	  // Keyword field, so that it's searchable, but so that no attempt is made
	  // to tokenize the field into words.
	  //doc.add(Field.Keyword("modified", DateField.timeToString(f.lastModified())));

	  doc.add(Field.Text("body", getBody()));

	  // return the document
	  return doc;
	}
}

class ArticleIterator {

	private String _prevTitle;
	private BufferedReader _reader;
	public ArticleIterator(String fileName)
	{
		System.out.println("using file " + fileName);
		_prevTitle = null;
		try {
			FileInputStream is = new FileInputStream(fileName);
			_reader = new BufferedReader(new InputStreamReader(is));
		} catch (Exception e) {
			System.out.println(" caught a " + e.getClass() + "\n with message: " + e.getMessage());
		}
	}

	public Article getNextArticle() throws Exception
	{
		String title,body,line;

		if (_prevTitle!=null)
		{
			title = _prevTitle;
		}
		else
		{
			title = _reader.readLine();
			if (null == title)
				return null;
		}

		String ns = _reader.readLine();

		StringBuffer strTmp = new StringBuffer(500);
		while (true)
		{
			line = _reader.readLine();
			if (null==line)
				break;

			if (line.startsWith("^"))
			{
				_prevTitle = line;
				break;
			}
			strTmp.append(line);
		}

		if (null==line)
		{
			// not exactly correct: we loose the last article
			return null;
		}
		body = new String(strTmp);
		return new Article(title, body);
	}

}

class testLupy {

	public static void Usage() {
		System.out.println("Usage: Index fileName");
	}

	public static void main(String[] args) {

		if (args.length != 1)
		{
			Usage();
			return;
		}

		String fileName = args[0];

		ArticleIterator aIter = new ArticleIterator(fileName);
	    try {
			Date start = new Date();

			IndexWriter writer = new IndexWriter("index", new StandardAnalyzer(), true);

			Article article;
			int count = 0;
			while(true)
			{
				article = aIter.getNextArticle();
				if (null==article)
				{
					System.out.println("getNextArticle() returned null. This is the end.");
					break;
				}
						
				indexArticle(writer, article);
				count += 1;
				if (count % 1000 == 0)
				{
					System.out.println("processed " + count + " articles");
				}
			}

			writer.optimize();
			writer.close();

			Date end = new Date();

			System.out.print(end.getTime() - start.getTime());
			System.out.println(" total milliseconds");

			} catch (Exception e) {
				System.out.println(" caught a " + e.getClass() +
					 "\n with message: " + e.getMessage());
			}
	}

	public static void indexArticle(IndexWriter writer, Article article) throws Exception
	{
		// System.out.println("adding " + article.getTitle());
		writer.addDocument(article.Document());
	}
}

